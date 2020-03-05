#include "../bounded_string.c"
// MARK Not a big fan of needing to include two C files, makes this somewhat
// of an integration test
#include "../streamed_string.c"

#include <fcntl.h>
#include <underscore.h>

UNIT(bounded_from_streamed_string) {
	SPEC("converts small strings") {
		FileDescriptor file = open("src/server/string/_/23B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 23B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		OKAY(bounded_from_streamed_string(&string, &bounded));
		ASSERT(bounded.length == 23);
		ASSERT(bounded.data[0] == '[' && bounded.data[22] == ']');

		free_streamed_string(&string);
		DONE;
	}
	SPEC("converts large strings") {
		FileDescriptor file = open("src/server/string/_/4098B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 4098B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		OKAY(bounded_from_streamed_string(&string, &bounded));
		ASSERT(bounded.length == 4098);
		ASSERT(bounded.data[0] == '[' && bounded.data[4097] == ']');

		free_streamed_string(&string);
		DONE;
	}
	SPEC("converts 0-length strings") {
		FileDescriptor file = open("src/server/string/_/0B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 0B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		OKAY(bounded_from_streamed_string(&string, &bounded));
		ASSERT(bounded.length == 0);

		free_streamed_string(&string);
		DONE;
	}
}

DRIVER {
	TEST(bounded_from_streamed_string);
}