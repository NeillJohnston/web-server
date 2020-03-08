#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Clean up dynamically-allocated memory and return ERROR_MALFORMED_REQUEST.
Modifies request and performs free operations depending on stage.

Helper for parse_http_request, when an error is detected it gets passed here.
*/
static ErrorCode clean_partial_parse(HttpRequest* request, Int stage) {
	if (stage <= 1) {
	}
	if (stage <= 2) {
		free(request->headers);
	}
	if (stage <= 3) {
	}
	if (stage <= 3) {
	}

	return ERROR_MALFORMED_REQUEST;
}

ErrorCode parse_http_request(BoundedString request_string, HttpRequest* request) {
	ErrorCode attempt_copy = copy_bounded_string(request_string, &request->raw);
	if (attempt_copy != 0) return -1;

	// Copy of raw so that we can pop lines/tokens and use it as backing data
	BoundedString raw = request->raw;

	{ // Stage 1: parse the first line - method, path, and version
		BoundedString line = pop_line_inplace(&raw);
		if (line.length == 0) return clean_partial_parse(request, 1);

		request->method = pop_token_inplace(&line);
		if (request->method.length == 0) return clean_partial_parse(request, 1);

		request->path = pop_token_inplace(&line);
		if (request->path.length == 0) return clean_partial_parse(request, 1);

		BoundedString version_string = pop_token_inplace(&line);
		if (version_string.length == 0) return clean_partial_parse(request, 1);
		Int n_version_tokens = sscanf(
			version_string.data, "HTTP/%u.%u",
			&request->version.major, &request->version.minor
		);
		if (n_version_tokens != 2) return clean_partial_parse(request, 1); 
	}
	{ // Stage 2: count headers, allocate space for header array
		// Save raw first so we can return to our previous state next stage
		BoundedString _raw = raw;
		request->n_headers = 0;

		while (true) {
			BoundedString line = pop_line_inplace(&raw);
			if (line.length == 0) break;
			++request->n_headers;
		}
		request->headers = (HttpHeader*) calloc(request->n_headers, sizeof(HttpHeader));

		raw = _raw;
	}
	{ // Stage 3: parse headers
		for (Size i = 0; i < request->n_headers; ++i) {
			BoundedString line = pop_line_inplace(&raw);

			BoundedString name = pop_token_inplace(&line);
			if (name.length == 0) return clean_partial_parse(request, 3);
			if (name.data[name.length-1] != ':') return clean_partial_parse(request, 3);
			--name.length;

			// MARK allows 0-length header values, haven't found evidence that
			// this isn't allowed
			BoundedString value = line;

			request->headers[i] = (HttpHeader) {
				.name = name,
				.value = value
			};
		}
	}
	{ // Stage 4: parse content
		if (raw.length == 0) return clean_partial_parse(request, 4);
		pop_line_inplace(&raw);
		request->content = raw;
	}

	return 0;
}

Void free_http_request(HttpRequest request) {
	free(request.headers);
	free_bounded_string(request.raw);
}