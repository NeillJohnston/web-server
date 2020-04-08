#include "../files.c"
#include "../../string/bounded_string.c"
#include "../../string/streamed_string.c"

#include <string.h>
#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

UNIT(get_file_contents) {
	SPEC("gets the contents of an existing file") {
		BoundedString path = make_bounded_string("src/server/files/_/exists.txt");
		BoundedString contents;

		OKAY(get_file_contents(path, &contents));
		ASSERT(strcmp(contents.data, "exists") == 0);
		
		DONE;
	}
	SPEC("errors for non-existent files") {
		BoundedString path = make_bounded_string("src/server/files/_/missing.txt");
		BoundedString contents;

		ASSERT(get_file_contents(path, &contents) == ERROR_NO_FILE);

		DONE;
	}
}

DRIVER {
	TEST(get_file_contents);
}

// Blank stubs
Int SSL_read_ex(SSL* ssl, Void* buf, Size num, Size *read_bytes) {
	return 0;
}