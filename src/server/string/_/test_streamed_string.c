#include "../streamed_string.c"

#include <fcntl.h>
#include <underscore.h>
#include <unistd.h>

static UInt count_nodes(StreamedString* string) {
	StreamedStringNode* current = &string->head;
	UInt count = 0;

	while (current != NULL) {
		++count;
		current = current->next;
	}

	return count;
}

static UInt count_bytes(StreamedString* string) {
	StreamedStringNode* current = &string->head;
	UInt count = 0;

	while (current != NULL) {
		count += current->size;
		current = current->next;
	}

	return count;
}

static bool all_xs(StreamedString* string) {
	StreamedStringNode* current = &string->head;
	
	while (current != NULL) {
		for (Size i = 0; i < current->size; ++i)
			if (current->data[i] != 'x')
				return false;
		current = current->next;
	}

	return true;
}

UNIT(read_streamed_string) {
	SPEC("reads small (<= NODE_SIZE) strings") {
		FileDescriptor file = open("src/server/string/_/23B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 23B.txt");

		StreamedString string;
		OKAY(read_streamed_string(file, &string));
		ASSERT(count_nodes(&string) == 1);
		ASSERT(count_bytes(&string) == 23);

		close(file);
		DONE;
	}
	SPEC("reads large (> NODE_SIZE) strings") {
		FileDescriptor file = open("src/server/string/_/4098B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 4098B.txt");

		StreamedString string;
		OKAY(read_streamed_string(file, &string));
		ASSERT(count_nodes(&string) > 1);
		ASSERT(count_bytes(&string) == 4098);

		close(file);
		DONE;
	}
	SPEC("reads data correctly") {
		FileDescriptor file = open("src/server/string/_/xs.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open xs.txt");

		StreamedString string;
		OKAY(read_streamed_string(file, &string));
		ASSERT(all_xs(&string));

		close(file);
		DONE;
	}
	SPEC("reads 0-length strings") {
		FileDescriptor file = open("src/server/string/_/0B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 0B.txt");

		StreamedString string;
		OKAY(read_streamed_string(file, &string));
		ASSERT(count_nodes(&string) == 1);
		ASSERT(count_bytes(&string) == 0);

		DONE;
	}
}

UNIT(free_streamed_string) {
	SPEC("frees small strings without error") {
		FileDescriptor file = open("src/server/string/_/23B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 23B.txt");

		StreamedString string;
		OKAY(read_streamed_string(file, &string));
		free_streamed_string(&string);
		// If there's an error the program will just crash

		close(file);
		DONE;
	}
	SPEC("frees large strings without error") {
		FileDescriptor file = open("src/server/string/_/4098B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 4098B.txt");

		StreamedString string;
		OKAY(read_streamed_string(file, &string));
		free_streamed_string(&string);
		// If there's an error the program will just crash

		close(file);
		DONE;
	}
}

DRIVER {
	TEST(read_streamed_string);
	TEST(free_streamed_string);
}