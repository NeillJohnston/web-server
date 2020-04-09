#include "dynamic.h"

#include <stdarg.h>
#include <string.h>

/*
A default no-data string, empty JSON array.
Used for statements that legitimately return no data and failures (get_json has to return something).
*/
static BoundedString no_data() {
	const Char* source = "[]";

	Size length = strlen(source);
	Char* data = malloc(length);
	if (data == NULL) return (BoundedString) { .data = NULL, .length = 0 };

	memcpy(data, source, length);
	return (BoundedString) { .data = data, .length = length };
}

/*
Get a C-string by appending multiple C-strings together
*/
static Char* concatenate(Size n, ...) {
	va_list args1, args2;
	va_start(args1, n);
	va_start(args2, n);

	Size length = 0;
	for (Size i = 0; i < n; ++i) length += strlen(va_arg(args1, Char*));

	Char* string = malloc(length);
	if (string == NULL) return NULL;
	Size pos = 0;
	for (Size i = 0; i < n; ++i) {
		Char* item = va_arg(args2, Char*);
		strcpy(string+pos, item);
		pos += strlen(item);
	}

	va_end(args1);
	va_end(args2);

	return string;
}

/*
Vector-style append.
Doubles capacity and reallocates if the suffix can't fit.
Since realloc is being called, this may error.
*/
static ErrorCode vector_append(Char* suffix, BoundedString* string, Size* capacity) {
	Size suffix_length = strlen(suffix);
	while (string->length + suffix_length > *capacity) {
		*capacity = (*capacity) * 2;
		string->data = realloc(string->data, *capacity);
		if (string->data == NULL) return -1;
	}

	append_cstr_inplace(suffix, string);
	
	return 0;
}

BoundedString get_json(sqlite3_stmt* query) {
	Int sql_code = SQLITE_BUSY;
	while (sql_code == SQLITE_BUSY) sql_code = sqlite3_step(query);
	if (sql_code == SQLITE_DONE) return no_data();

	BoundedString json = {
		.data = malloc(1),
		.length = 0
	};
	// Handled vector-style, i.e. double every time we need more length
	Size capacity = 1;

	if (vector_append("[", &json, &capacity) != 0) return no_data();

	while (sql_code == SQLITE_ROW) {
		if (vector_append("{", &json, &capacity) != 0) return no_data();

		Size n_columns = (Size) sqlite3_column_count(query);
		for (Int i = 0; i < n_columns; ++i) {
			const Char* name = (Char*) sqlite3_column_name(query, i);
			Int type = sqlite3_column_type(query, i);

			if (type == SQLITE_INTEGER) {
				Int int_value = sqlite3_column_int(query, i);
				Char value [16];
				sprintf(value, "%d", int_value);

				Char* row = concatenate(5, "\"", name, "\":", value, ",");
				if (row == NULL) return no_data();

				if (vector_append(row, &json, &capacity) != 0) return no_data();
			}
			else if (type == SQLITE_TEXT) {
				const Char* value = (Char*) sqlite3_column_text(query, i);

				Char* row = concatenate(5, "\"", name, "\":\"", value, "\",");
				if (row == NULL) return no_data();

				if (vector_append(row, &json, &capacity) != 0) return no_data();
			}
			else {
				// Unsupported type
				// TODO: support more types as necessary
				return no_data();
			}
		}

		// Remove last comma
		if (n_columns > 0) --json.length;
		if (vector_append("},", &json, &capacity) != 0) return no_data();

		sql_code = sqlite3_step(query);
	}

	// Remove last comma
	--json.length;
	if (vector_append("]", &json, &capacity) != 0) return no_data();

	return json;
}