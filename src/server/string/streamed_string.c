#include "streamed_string.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ErrorCode read_streamed_string(FileDescriptor stream, StreamedString* string) {
	const Size data_size = sizeof string->head.data;
	StreamedStringNode* current = &string->head;

	while (true) {
		current->next = NULL;
		SSize n_bytes = read(stream, current->data, data_size);

		if (n_bytes == -1) {
			free_streamed_string(string);
			return -1;
		}
		else if (n_bytes < data_size) {
			current->size = n_bytes;
			break;
		}
		else {
			StreamedStringNode* next = (StreamedStringNode*) malloc(sizeof(StreamedStringNode));
			if (next == NULL) {
				free_streamed_string(string);
				return -1;
			}
			
			current->next = next;
			current->size = n_bytes;
			current = next;
		}
	}
	
	return 0;
}

// MARK: copy-pasted from above, needs refactor
ErrorCode read_ssl_streamed_string(SSL* ssl, StreamedString* string) {
	const Size data_size = sizeof string->head.data;
	StreamedStringNode* current = &string->head;

	while (true) {
		current->next = NULL;
		Int attempt_read = SSL_read(ssl, current->data, data_size);
		if (attempt_read <= 0) {
			free_streamed_string(string);

			Int ssl_error = SSL_get_error(ssl, attempt_read);
			if (ssl_error == SSL_ERROR_SSL || ssl_error == SSL_ERROR_SYSCALL) return ERROR_INVALIDATED_SSL;
			else return -1;
		}
		
		Size n_bytes = attempt_read;

		if (n_bytes < data_size) {
			current->size = n_bytes;
			break;
		}
		else {
			StreamedStringNode* next = (StreamedStringNode*) malloc(sizeof(StreamedStringNode));
			if (next == NULL) {
				free_streamed_string(string);
				return -1;
			}
			
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
