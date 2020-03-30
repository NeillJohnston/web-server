#include "http.h"
#include "protocol/header_names.h"
#include "protocol/status_messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ErrorCode add_http_header(enum HeaderNameCode name_code, BoundedString value, HttpResponse* response) {
	++response->n_headers;
	response->headers = realloc(response->headers, response->n_headers * sizeof(HttpHeader));
	if (response->headers == NULL) return -1;
	HttpHeader* header = &response->headers[response->n_headers-1];

	header->name_code = name_code;
	header->value.data = malloc(value.length);
	header->value.length = value.length;
	if (header->value.data == NULL) return -1;

	memcpy(header->value.data, value.data, value.length);

	return 0;
}

ErrorCode make_http_response_string(HttpResponse response, BoundedString* response_string) {
	const Char* status_message = STATUS_MESSAGES[response.status_code];

	// Find out about how much space we need so we can malloc first
	// No need to be stingy with our bytes, so the quick estimate formula:
	// 1KB + |status_message| + sum(|header|) + |content|
	// MARK: just in case the above changes and we need to be stingy
	Size length = 1024;
	length += strlen(status_message);
	for (Size i = 0; i < response.n_headers; ++i)
		length += strlen(HEADER_NAMES[response.headers[i].name_code]) + response.headers[i].value.length;
	length += response.content.length;

	response_string->data = malloc(length);
	if (response_string->data == NULL) return -1;
	response_string->length = 0;

	// Constants so we can just keep using append_inplace
	const BoundedString NEWLINE = { .data = "\n", .length = 1 };
	const BoundedString HEADER_SEP = { .data = ": ", .length = 2 };

	response_string->length += sprintf(
		response_string->data,
		"HTTP/%u.%u %u %s",
		response.version.major, response.version.minor, response.status_code, status_message
	);
	append_inplace(NEWLINE, response_string);

	for (Size i = 0; i < response.n_headers; ++i) {
		append_cstr_inplace(HEADER_NAMES[response.headers[i].name_code], response_string);
		append_inplace(HEADER_SEP, response_string);
		append_inplace(response.headers[i].value, response_string);
		append_inplace(NEWLINE, response_string);
	}
	append_inplace(NEWLINE, response_string);

	append_inplace(response.content, response_string);

	return 0;
}

Void free_http_response(HttpResponse response) {
	for (Size i = 0; i < response.n_headers; ++i)
		free_bounded_string(response.headers[i].value);
	free(response.headers);

	free_bounded_string(response.content);
}