#include "bounded_string.h"
#include "streamed_string.h"

#include <stdlib.h>
#include <string.h>

ErrorCode bounded_from_streamed_string(StreamedString* string, BoundedString* bounded) {
	StreamedStringNode* current;
	bounded->length = 0;
	
	current = &string->head;
	while (current != NULL) {
		bounded->length += current->size;
		current = current->next;
	}
	bounded->data = malloc(bounded->length);
	if (bounded->data == NULL) return -1;

	current = &string->head;
	Size back = 0;
	while (current != NULL) {
		memcpy(bounded->data+back, current->data, current->size);
		back += current->size;
		current = current->next;
	}

	return 0;
}

Void free_bounded_string(BoundedString string) {
	free(string.data);
}