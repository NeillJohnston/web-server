#pragma once

#include "../string/bounded_string.h"

typedef struct {
	UInt major;
	UInt minor;
} HttpVersion;

typedef struct {
	BoundedString name;
	BoundedString value;
} HttpHeader;

typedef struct {
	BoundedString method;
	BoundedString path;
	HttpVersion version;

	Size n_headers;
	HttpHeader* headers;
	
	BoundedString content;

	BoundedString raw;
} HttpRequest;

typedef struct {
	HttpVersion version;
	UInt status_code;
	BoundedString status_message;

	Size n_headers;
	HttpHeader* headers;

	BoundedString content;
} HttpResponse;

/*
Turn a raw request string into a structured HTTP response.
Can error in the case of a malformed request.
Writes back to request.

Some parts of request will be dynamically allocated.
Both request_string and request can be freed independently after calling.
*/
ErrorCode parse_http_request(BoundedString request_string, HttpRequest* request);

/*
Turn a structured HTTP response into a raw response string.

The data of the resulting BoundedString will be dynamically allocated.
Both the resulting BoundedString and request can be freed independently after
calling.
*/
BoundedString make_http_response_string(HttpResponse response);

/*
Free the data of an HttpRequest.
*/
Void free_http_request(HttpRequest request);

/*
Free the data of an HttpResponse.
*/
Void free_http_response(HttpResponse response);