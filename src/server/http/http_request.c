#include "http.h"
#include "protocol/header_names.h"
#include "protocol/method_names.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Turn a string into a method code.
*/
static ErrorCode parse_method(BoundedString method, enum MethodCode* method_code) {
	// MARK: enum abuse
	// Might be worth it to include dummy iterator values in the enum:
	// for (enum MethodCode i = METHOD_CODE_FIRST+1; i != METHOD_CODE_LAST; ++i)
	for (enum MethodCode i = GET; i <= CONNECT; ++i) {
		if (bounded_string_equ_cstr(method, METHOD_NAMES[i])) {
			*method_code = i;
			return 0;
		}
	}

	return -1;
}

/*
Consume and parse the request line for an HTTP request.
*/
static ErrorCode parse_request_line(BoundedString* request_string, HttpRequest* request) {
	BoundedString line = pop_line_inplace(request_string);
	BoundedString method = pop_token_inplace(&line);
	if (method.length == 0) return -1;
	BoundedString uri = pop_token_inplace(&line);
	if (uri.length == 0) return -1;
	BoundedString version_string = pop_token_inplace(&line);
	if (version_string.length == 0) return -1;

	if (parse_method(method, &request->method_code) != 0) return -1;

	if (bounded_string_equ_cstr(uri, "*"))
		request->request_uri.type = ASTERISK;
	else if (uri.data[0] == '/')
		request->request_uri.type = ABS_PATH;
	else
		request->request_uri.type = ABSOLUTE;
	request->request_uri.uri = uri;

	if (2 != sscanf(version_string.data, "HTTP/%d.%d", &request->version.major, &request->version.minor)) {
		return -1;
	}

	return 0;
}

/*
Turn a string into a header name code.
*/
static ErrorCode parse_header_name(BoundedString header_name, enum HeaderNameCode* header_name_code) {
	for (enum HeaderNameCode i = ITER_HEADER_NAME_CODES+1; i != END_HEADER_NAME_CODES; ++i) {
		if (bounded_string_equ_cstr(header_name, HEADER_NAMES[i])) {
			*header_name_code = i;
			return 0;
		}
	}

	return -1;
}

/*
Consume and parse the headers for an HTTP request.
Assumes parse_request_line has run successfully.

Supported headers are declared/defined in enum HeaderNameCodes and HEADER_NAMES.
Any unsupported headers should just be cut from the final message, but
malformed headers should trigger an error.
*/
static ErrorCode parse_headers(BoundedString* request_string, HttpRequest* request) {
	// Count headers first so we only have to alloc once
	request->n_headers = 0;
	request->headers = NULL;
	while (true) {
		BoundedString line = pop_line_inplace(request_string);
		if (line.length == 0) break;

		Size name_end = 0;
		while (line.data[name_end] != ':' && name_end < line.length)
			++name_end;
		if (name_end == line.length) return -1;
		
		BoundedString name = {
			.data = line.data,
			.length = name_end
		};
		enum HeaderNameCode name_code;

		if (parse_header_name(name, &name_code) != 0) continue;

		BoundedString value = {
			.data = line.data + (name_end + 1),
			.length = line.length - (name_end + 1)
		};
		trim_inplace(&value);

		++request->n_headers;
		request->headers = realloc(request->headers, request->n_headers * sizeof(HttpHeader));
		if (request->headers == NULL) return -1;
		request->headers[request->n_headers-1] = (HttpHeader) {
			.name_code = name_code,
			.value = value
		};
	}

	return 0;
}

ErrorCode parse_http_request(BoundedString request_string, HttpRequest* request) {
	if (copy_bounded_string(request_string, &request->raw) != 0) return -1;
	
	BoundedString mutable_raw = request->raw;
	
	// Keep modifying the request string
	ErrorCode attempt;
	attempt = parse_request_line(&mutable_raw, request);
	if (attempt != 0) return attempt;
	attempt = parse_headers(&mutable_raw, request);
	if (attempt != 0) return attempt;
	
	// The rest is the message body (content)
	// TODO: check standard and see if Content-Length has to be used
	request->content = mutable_raw;

	return 0;
}

Void free_http_request(HttpRequest request) {
	free(request.headers);
	free_bounded_string(request.raw);
}