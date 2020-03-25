#include "../http_request.c"

#include "../../string/bounded_string.c"

#include <string.h>
#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

Bool equ(BoundedString bounded, const Char* string) {
	Size length = strlen(string);
	if (bounded.length != length) return false;
	return strncmp(bounded.data, string, length) == 0;
}

UNIT(parse_request_line) {
	SPEC("parses valid request lines") {
		// ... is the rest of the message
		BoundedString get = make_bounded_string("GET / HTTP/1.1\r\n...");
		BoundedString post = make_bounded_string("POST localhost/api/test HTTP/1.0\r\n...");
		HttpRequest request;

		OKAY(parse_request_line(&get, &request));
		ASSERT(get.length == 3);
		ASSERT(request.method_code == GET);
		ASSERT(request.request_uri.type == ABS_PATH);
		COMPARE(request.request_uri.uri, equ, "/");
		ASSERT(request.version.major == 1 && request.version.minor == 1);

		OKAY(parse_request_line(&post, &request));
		ASSERT(post.length == 3);
		ASSERT(request.method_code == POST);
		ASSERT(request.request_uri.type == ABSOLUTE);
		COMPARE(request.request_uri.uri, equ, "localhost/api/test");
		ASSERT(request.version.major == 1 && request.version.minor == 0);

		DONE;
	}
	SPEC("errors for an invalid request line") {
		BoundedString bad_method = make_bounded_string("NO / HTTP/1.1\r\n...");
		BoundedString no_request_uri = make_bounded_string("GET HTTP/1.1\r\n...");
		BoundedString bad_version = make_bounded_string("GET / HTTP/10\r\n...");
		HttpRequest request;

		ERROR(parse_request_line(&bad_method, &request));
		ERROR(parse_request_line(&no_request_uri, &request));
		ERROR(parse_request_line(&bad_version, &request));

		DONE;
	}
}

UNIT(parse_headers) {
	SPEC("returns properly for zero headers") {
		BoundedString zero = make_bounded_string("\r\n...");
		HttpRequest request;

		OKAY(parse_headers(&zero, &request));
		ASSERT(zero.length == 3);
		ASSERT(request.n_headers == 0);

		DONE;
	}
	SPEC("parses multiple headers") {
		BoundedString two = make_bounded_string("Content-Type: text/json\r\nContent-Length: 4\r\n\r\n...");
		HttpRequest request;

		OKAY(parse_headers(&two, &request));
		ASSERT(two.length == 3);
		ASSERT(request.n_headers == 2);
		ASSERT(request.headers[0].name_code == CONTENT_TYPE);
		COMPARE(request.headers[0].value, equ, "text/json");
		ASSERT(request.headers[1].name_code == CONTENT_LENGTH);
		COMPARE(request.headers[1].value, equ, "4");

		DONE;
	}
	SPEC("discards unsupported headers") {
		BoundedString half_unsupported = make_bounded_string("Test: not supported\r\nAccept: text/html\r\n\r\n...");
		HttpRequest request;

		OKAY(parse_headers(&half_unsupported, &request));
		ASSERT(half_unsupported.length == 3);
		ASSERT(request.n_headers == 1);
		ASSERT(request.headers[0].name_code == ACCEPT);
		COMPARE(request.headers[0].value, equ, "text/html");

		DONE;
	}
	SPEC("errors for invalid headers") {
		BoundedString invalid = make_bounded_string("Content-Type: text/json\r\nthis aint no header\r\nContent-Length: 4\r\n\r\n...");
		HttpRequest request;

		ERROR(parse_headers(&invalid, &request));

		DONE;
	}
}

UNIT(parse_http_request) {
	SPEC("parses simple requests") {
		BoundedString request_string = make_bounded_string(
			"GET / HTTP/1.1\r\n"
			"User-Agent: Edge;Chromium\r\n"
			"\r\n"
		);
		HttpRequest request;

		OKAY(parse_http_request(request_string, &request));
		ASSERT(request.method_code == GET);
		ASSERT(request.request_uri.type == ABS_PATH);
		COMPARE(request.request_uri.uri, equ, "/");
		ASSERT(request.version.major == 1);
		ASSERT(request.version.minor == 1);
		ASSERT(request.n_headers == 1);
		ASSERT(request.headers[0].name_code == USER_AGENT);
		COMPARE(request.headers[0].value, equ, "Edge;Chromium");
		COMPARE(request.content, equ, "");
		ASSERT(bounded_string_equ(request.raw, request_string));

		DONE;
	}
	SPEC("errors for invalid requests") {
		BoundedString no_method = make_bounded_string(
			"/ HTTP/1.1\r\n"
			"User-Agent: Edge;Chromium\r\n"
			"\r\n"
		);
		BoundedString bad_headers = make_bounded_string(
			"/ HTTP/1.1\r\n"
			"User-Agent: Edge;Chromium\r\n"
			"No colon\r\n"
			"\r\n"
		);
		BoundedString bad_version = make_bounded_string(
			"GET / HTTP=1.1\r\n"
			"User-Agent: Edge;Chromium\r\n"
			"\r\n"
		);
		HttpRequest request;

		ERROR(parse_http_request(no_method, &request));
		ERROR(parse_http_request(bad_headers, &request));
		ERROR(parse_http_request(bad_version, &request));

		DONE;
	}
}

UNIT(free_http_request) {
	SPEC("has been tested with Valgrind") {
		ASSERT(false);
		DONE;
	}
}

DRIVER {
	TEST(parse_request_line);
	TEST(parse_headers);
	TEST(parse_http_request);
	TEST(free_http_request);
}