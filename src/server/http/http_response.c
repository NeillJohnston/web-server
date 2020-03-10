#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ErrorCode make_http_response_string(HttpResponse response, BoundedString* response_string) {
	// Find out about how much space we need so we can malloc first
	// No need to be stingy with our bytes, so the quick estimate formula:
	// 1KB + |status_message| + sum(|header|) + |content|
	// MARK just in case the above changes and we need to be stingy
	Size length = 1024;
	length += response.status_message.length;
	for (Size i = 0; i < response.n_headers; ++i)
		length += response.headers[i].name.length + response.headers[i].value.length;
	length += response.content.length;

	response_string->data = malloc(length);
	if (response_string->data == NULL) return -1;
	response_string->length = 0;

	// Constants so we can just keep using append_inplace
	const BoundedString NEWLINE = { .data = "\n", .length = 1 };
	const BoundedString HEADER_SEP = { .data = ": ", .length = 2 };

	response_string->length += sprintf(
		response_string->data,
		"HTTP/%u.%u %u ",
		response.version.major, response.version.minor, response.status_code
	);
	append_inplace(response.status_message, response_string);
	append_inplace(NEWLINE, response_string);

	for (Size i = 0; i < response.n_headers; ++i) {
		append_inplace(response.headers[i].name, response_string);
		append_inplace(HEADER_SEP, response_string);
		append_inplace(response.headers[i].value, response_string);
		append_inplace(NEWLINE, response_string);
	}
	append_inplace(NEWLINE, response_string);

	append_inplace(response.content, response_string);

	return 0;
}

Void free_http_response(HttpResponse response) {
	free_bounded_string(response.status_message);
	
	for (Size i = 0; i < response.n_headers; ++i) {
		free_bounded_string(response.headers[i].name);
		free_bounded_string(response.headers[i].value);
	}
	free(response.headers);

	free_bounded_string(response.content);
}