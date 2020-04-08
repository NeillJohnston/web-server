#include "../json.c"
#include "../../../string/bounded_string.c"

#include <underscore.h>

/*
EWISOTT
*/
Bool whitespace(Char c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/*
Returns whether two strings are equal, skipping over whitespace
*/
Bool content_equ(BoundedString src, BoundedString pat) {
	Size i = 0, j = 0;
	while (i < src.length && j < pat.length) {
		if (whitespace(src.data[i])) { ++i; continue; }
		if (whitespace(pat.data[j])) { ++j; continue; }
		if (src.data[i] != pat.data[j]) return false;
		++i;
		++j;
	}
	while (i < src.length && whitespace(src.data[i])) ++i;
	while (j < pat.length && whitespace(pat.data[j])) ++j;

	return i == src.length && j == pat.length;
}

UNIT(get_json) {
	SPEC("returns a JSON string with the right data for varchars and integer types") {
		sqlite3* db;
		if (sqlite3_open("src/server/router/dynamic/_/test.db", &db) != 0) LEAVE("could not open DB");
		sqlite3_stmt* stmt;
		const Char* query = "select * from DATA order by INT asc;";
		if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != 0) LEAVE("could not prepare statement");
		BoundedString match = {
			.data = "[{ \"STR\": \"\", \"INT\": 0 }, { \"STR\": \"one\", \"INT\": 1 }, { \"STR\": \"two\", \"INT\": 2 }]",
			.length = 81
		};

		BoundedString json = get_json(stmt);

		COMPARE(json, content_equ, match);

		DONE;
	}
	SPEC("returns a JSON string that may be an empty array") {
		sqlite3* db;
		if (sqlite3_open("src/server/router/dynamic/_/test.db", &db) != 0) LEAVE("could not open DB");
		sqlite3_stmt* stmt;
		const Char* query = "select * from DATA where INT = -1;";
		if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != 0) LEAVE("could not prepare statement");
		BoundedString match = {
			.data = "[]",
			.length = 2
		};

		BoundedString json = get_json(stmt);

		COMPARE(json, content_equ, match);

		DONE;
	}
}

DRIVER {
	TEST(get_json);
}