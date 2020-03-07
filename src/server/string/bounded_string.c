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

Bool bounded_string_equ(BoundedString x, BoundedString y) {
	if (x.length != y.length) return false;

	for (Size i = 0; i < x.length; ++i)
		if (x.data[i] != y.data[i])
			return false;
	
	return true;
}

BoundedString pop_line_inplace(BoundedString* string) {
	BoundedString line = {
		.data = string->data
	};

	for (Size i = 0; i < string->length; ++i) {
		// LF
		if (string->data[i] == '\n') {
			line.length = i;
			string->length -= i+1;
			string->data += i+1;
			return line;
		}
		// CRLF
		else if (string->data[i] == '\r') {
			if (i < string->length-1 && string->data[i+1] == '\n') {
				line.length = i;
				string->length -= i+2;
				string->data += i+2;
				return line;
			}
		}
	}

	// Original string is one line
	line.length = string->length;
	string->length = 0;
	return line;
}

static Bool is_whitespace(Char character) {
	return character == ' ' || character == '\t' || character == '\r' || character == '\n';
}

BoundedString pop_token_inplace(BoundedString* string) {
	BoundedString token = {
		.data = string->data
	};

	for (Size i = 0; i < string->length; ++i) {
		if (is_whitespace(string->data[i])) {
			token.length = i;
			
			while (i < string->length && is_whitespace(string->data[i]))
				++i;
			string->data += i;
			string->length -= i;

			return token;
		}
	}

	// Original string is one token
	token.length = string->length;
	string->length = 0;
	return token;
}

Void free_bounded_string(BoundedString string) {
	free(string.data);
}