#include "../streamed_string.c"

#include <fcntl.h>
#include <stdio.h>
#include <underscore.h>
#include <unistd.h>

static int count_nodes(StreamedString* string) {
	StreamedStringNode* current = &string->head;
	int count = 0;

	while (current != NULL) {
		++count;
		current = current->next;
	}

	return count;
}

static int count_bytes(StreamedString* string) {
	StreamedStringNode* current = &string->head;
	int count = 0;

	while (current != NULL) {
		count += current->n_bytes;
		current = current->next;
	}

	return count;
}

UNIT(read_streamed_string) {
	// Initial tests to make sure strings can be read -
	// We'll test harder when we can actually have the string data
	SPEC("reads small (<= NODE_SIZE) strings") {
		FileDescriptor file = open("src/server/string/_test/23B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 23B.txt");

		StreamedString string;

		OKAY(read_streamed_string(file, &string));
		ASSERT(count_nodes(&string) == 1);
		ASSERT(count_bytes(&string) == 23);

		close(file);
		DONE;
	}
	SPEC("reads large (> NODE_SIZE) strings") {
		FileDescriptor file = open("src/server/string/_test/4098B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 4098B.txt");

		StreamedString string;

		OKAY(read_streamed_string(file, &string));
		ASSERT(count_nodes(&string) > 1);
		ASSERT(count_bytes(&string) == 4098);

		close(file);
		DONE;
	}
}

DRIVER {
	TEST(read_streamed_string);
}