#include "dynamic.h"
#include "../../files/files.h"

#include <linux/limits.h>
#include <string.h>

/*
Write a bounded string to a C-string.
Assumes the C-string has an adequate buffer size to hold both the string and the null-terminator.
MARK: Not DRY, copied from router.c
*/
static Void write_bounded_to_cstr(BoundedString string, Char* cstr) {
	memcpy(cstr, string.data, string.length);
	cstr[string.length] = '\0';
}

UInt route_dynamic(BoundedString root, BoundedString database_path, BoundedString path, HttpResponse* response) {
	sqlite3* database;
	Char database_path_cstr [PATH_MAX];
	if (database_path.length+1 > PATH_MAX) return 500;
	write_bounded_to_cstr(database_path, database_path_cstr);
	if (sqlite3_open(database_path_cstr, &database) != 0) return 500;

	Char full_path_data [PATH_MAX];
	BoundedString full_path = {
		.data = full_path_data,
		.length = 0
	};
	if (root.length + path.length + 1 > PATH_MAX) return 500;
	append_inplace(root, &full_path);
	append_inplace(path, &full_path);

	BoundedString query;
	// No need to 404 on file not found, that's a router problem
	if (get_file_contents(full_path, &query) != 0) return 500;

	Char* query_cstr = malloc(query.length+1);
	if (query_cstr == NULL) return 500;
	write_bounded_to_cstr(query, query_cstr);

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(database, query_cstr, -1, &stmt, NULL)) return 500;

	response->content = get_json(stmt);

	// TODO: needs application/json MIME type

	sqlite3_finalize(stmt);
	sqlite3_close(database);

	free_bounded_string(query);
	free(query_cstr);

	return 200;
}