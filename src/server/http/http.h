#pragma once

#include "../string/bounded_string.h"

static const ErrorCode ERROR_MALFORMED_REQUEST = 1;

/*
Structured form of an HTTP version.
*/
typedef struct {
	UInt major;
	UInt minor;
} HttpVersion;

/*
Structured form of an HTTP header.
*/
typedef struct {
	BoundedString name;
	BoundedString value;
} HttpHeader;

/*
Structured form of an HTTP request.
Includes all the necessary data (for HTTP version < 2) and the original string
the request comes from (raw).

MARK might be nice to have an enum for method instead of passing magic strings
around
MARK might be useful to have an array type for HttpHeader, instead of breaking
it into two fields - figure out during implementation if this is necessary
*/
typedef struct {
	BoundedString method;
	BoundedString path;
	HttpVersion version;

	Size n_headers;
	HttpHeader* headers;
	
	BoundedString content;

	BoundedString raw;
} HttpRequest;

/*
Structured form of an HTTP response.
Includes all the necessary data (for HTTP version < 2) and the original string
the request comes from (raw).

MARK see HttpRequest, same comment about headers
*/
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