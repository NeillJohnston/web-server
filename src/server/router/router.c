// MARK: This entire C file is ~200 lines and implements ONE FUNCTION for
// which it has ONE UNIT TEST.
// Granted, the presence of SQLite makes it hard to test, but there's still
// a lot of untested code being used.

#include "router.h"
#include "dynamic/dynamic.h"
#include "static/static.h"
#include "../http/protocol/method_names.h"

#include <linux/limits.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
SQL query to find a route with a given method/route in the database.
MAGIC, but it's genuinely an okay way to do things this time.
*/
static const Char* FIND_ROUTE =
"select * from routes where method = ? and route = ?;";

/*
SQL query to find an error route.
MAGIC, but it's genuinely an okay way to do things this time.
*/
static const Char* FIND_ERROR_ROUTE =
"select * from routes where method = \"ERROR\" and route = ?;";

/*
Default blank response, importantly has all pointers NULL so that free
can be called without error.
*/
static const HttpResponse BLANK_RESPONSE = {
	.version = {
		.major = 1,
		.minor = 1
	},
	.status_code = 500,
	.n_headers = 0,
	.headers = NULL,
	.content = {
		.data = NULL,
		.length = 0
	}
};

/*
Make a default 500 response.
*/
static HttpResponse make_500() {
	const Char* content = "Internal Error";
	const Size content_length = strlen(content);
	HttpResponse response = BLANK_RESPONSE;

	// Have to dynamically allocate this string so that free can be called
	// "Rain or shine," make sure something useful can be returned here
	response.content.data = malloc(content_length);
	response.content.length = response.content.data == NULL ? 0 : content_length;

	return response;
}

/*
Write a bounded string to a C-string.
Assumes the C-string has an adequate buffer size to hold both the string and the null-terminator.
*/
static Void write_bounded_to_cstr(BoundedString string, Char* cstr) {
	memcpy(cstr, string.data, string.length);
	cstr[string.length] = '\0';
}

/*
Wait for SQLite statement to return a non-busy code.
*/
static Int first_row(sqlite3_stmt* statement) {
	Int sql_code = SQLITE_BUSY;
	while (sql_code == SQLITE_BUSY)
		sql_code = sqlite3_step(statement);
	return sql_code;
}

/*
Read the data (path/type) columns from a row in the routes table.
*/
static Void read_routes_row(sqlite3_stmt* statement, BoundedString* path, Int* type) {
	const unsigned char* path_data = sqlite3_column_text(statement, 2);
	path->length = (Size) sqlite3_column_bytes(statement, 2);
	memcpy(path->data, path_data, path->length);

	*type = sqlite3_column_int(statement, 3);
}

/*
Route to a requested path, writing back to response.
Proxy for all the sub-routers.
*/
static Void route(BoundedString path, BoundedString params, Int type, ServerConfig config, HttpResponse* response) {
	if (type == ROUTE_STATIC) {
		response->status_code = route_static(config.root, path, response);
	}
	else if (type == ROUTE_DYNAMIC) {
		response->status_code = route_dynamic(config.root, config.db_path, path, params, response);
	}
}

HttpResponse respond(HttpRequest request, ServerConfig config) {
	Char buffer [PATH_MAX];
	HttpResponse response = BLANK_RESPONSE;
	sqlite3* database;
	
	if (add_blank_http_header(CONTENT_TYPE, &response) != 0) return make_500();

	const Char* method = METHOD_NAMES[request.method_code];
	BoundedString params = request.request_uri.uri;
	BoundedString uri = pop_delimited_inplace(&params, '?');

	write_bounded_to_cstr(config.db_path, buffer);
	if (sqlite3_open(buffer, &database) != 0) return make_500();

	write_bounded_to_cstr(uri, buffer);
	sqlite3_stmt* find_route;
	if (sqlite3_prepare_v2(database, FIND_ROUTE, -1, &find_route, NULL) != 0) return make_500();
	if (sqlite3_bind_text(find_route, 1, method, -1, SQLITE_STATIC) != 0) return make_500();
	if (sqlite3_bind_text(find_route, 2, uri.data, uri.length, SQLITE_STATIC) != 0) return make_500();
	
	if (first_row(find_route) == SQLITE_ROW) {
		Char path_data [PATH_MAX];
		BoundedString path = { .data = path_data };
		Int type;
		
		read_routes_row(find_route, &path, &type);
		sqlite3_finalize(find_route);
		route(path, params, type, config, &response);
	}
	// No route found, try static routing to the request URI
	else {
		sqlite3_finalize(find_route);
		route(uri, params, ROUTE_STATIC, config, &response);
	}

	// Try re-routing for errors
	if (response.status_code != 200) {
		sprintf(buffer, "%u", response.status_code);

		sqlite3_stmt* find_error_route;
		if (sqlite3_prepare_v2(database, FIND_ERROR_ROUTE, -1, &find_error_route, NULL) != 0) return make_500();
		if (sqlite3_bind_text(find_error_route, 1, buffer, -1, SQLITE_STATIC) != 0) return make_500();
		
		if (first_row(find_error_route) == SQLITE_ROW) {
			Char path_data [PATH_MAX];
			BoundedString path = { .data = path_data };
			Int type;

			read_routes_row(find_error_route, &path, &type);
			sqlite3_finalize(find_error_route);

			UInt status_code = response.status_code;
			route(path, params, type, config, &response);
			response.status_code = status_code;
		}
	}

	sqlite3_close(database);

	// Content length header
	sprintf(buffer, "%lu", response.content.length);
	BoundedString content_length = { .data = buffer, .length = strlen(buffer) };
	if (add_http_header(CONTENT_LENGTH, content_length, &response) != 0) return make_500();

	return response;
}