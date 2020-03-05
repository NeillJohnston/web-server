#include "streamed_string.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ErrorCode read_streamed_string(FileDescriptor stream, StreamedString* string) {
	const Size data_size = sizeof string->head.data;
	StreamedStringNode* current = &string->head;

	while (true) {
		SSize n_bytes = read(stream, current->data, data_size);

		if (n_bytes == -1) return -1;
		else if (n_bytes < data_size) {
			current->next = NULL;
			current->size = n_bytes;
			break;
		}
		else {
			StreamedStringNode* next = (StreamedStringNode*) malloc(sizeof(StreamedStringNode));
			if (next == NULL) return -1;
			
			current->next = next;
			current->size = n_bytes;
			current = next;
		}
	}
	
	return 0;
}

// TODO: Finish testing with Valgrind or something
Void free_streamed_string(StreamedString* string) {
	StreamedStringNode* current = (string->head).next;

	while (current != NULL) {
		StreamedStringNode* next = current->next;
		free(current);
		current = next;
	}
}