#include "streamed_string.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

ErrorCode read_streamed_string(FileDescriptor stream, StreamedString* string) {
	typedef ssize_t SSize;

	const Size data_size = sizeof string->head.data;
	StreamedStringNode* current = &string->head;

	while (true) {
		SSize n_bytes = read(stream, current->data, data_size);
		
		if (n_bytes == -1) return -1;
		else if (n_bytes < data_size) {
			current->next = NULL;
			current->n_bytes = n_bytes;
			break;
		}
		else {
			StreamedStringNode* next = (StreamedStringNode*) malloc(sizeof(StreamedStringNode));
			current->next = next;
			current->n_bytes = n_bytes;
			current = next;
		}
	}
	
	return 0;
}
