#include "dynamic.h"
#include "../../files/files.h"

#include <string.h>

UInt route_dynamic(BoundedString root, BoundedString database_path, BoundedString path, BoundedString params, HttpResponse* response) {
	sqlite3* database;
	Char* database_path_cstr;
	if (copy_bounded_string_to_cstr(database_path, &database_path_cstr) != 0) return 500;
	if (sqlite3_open(database_path_cstr, &database) != 0) return 500;

	Char* full_path_data = malloc(root.length + path.length);
	if (full_path_data == NULL) return 500;
	BoundedString full_path = { .data = full_path_data, .length = 0 };
	append_inplace(root, &full_path);
	append_inplace(path, &full_path);

	BoundedString query;
	Char* query_cstr;
	// No need to 404 on file not found, that's technically a router problem
	if (get_file_contents(full_path, &query) != 0) return 500;
	if (copy_bounded_string_to_cstr(query, &query_cstr) != 0) return 500;

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(database, query_cstr, -1, &stmt, NULL) != SQLITE_OK) return 500;

	// Bind URL query params
	// MARK: shoddy support for "typical" param syntax because I don't intend to use
	// this for anything like HTML forms
	for (Int i = 1; i <= sqlite3_bind_parameter_count(stmt); ++i) {
		BoundedString param = pop_delimited_inplace(&params, ',');
		Char* param_cstr;
		if (copy_bounded_string_to_cstr(param, &param_cstr) != 0) return 500;

		if (sqlite3_bind_text(stmt, i, param_cstr, -1, free) != SQLITE_OK) return 500;

		free(param_cstr);
	}

	response->content = get_json(stmt);

	// Content type header
	HttpHeader* header;
	if (get_http_response_header(CONTENT_TYPE, *response, &header) != 0) return 500;

	BoundedString content_type = { .data = "application/json", .length = 16 };
	if (copy_bounded_string(content_type, &header->value) != 0) return 500;

	sqlite3_finalize(stmt);
	sqlite3_close(database);

	free(database_path_cstr);
	free_bounded_string(full_path);
	free_bounded_string(query);
	free(query_cstr);

	return 200;
}