#include "../http_response.c"

#include "../../string/bounded_string.c"

#include <string.h>
#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

BoundedString make_dynamic_bounded_string(Char* data) {
	BoundedString string;
	string.length = strlen(data);
	string.data = malloc(string.length);
	memcpy(string.data, data, string.length);
	return string;
}

Bool equ(BoundedString bounded, const Char* string) {
	Size length = strlen(string);
	if (bounded.length != length) return false;
	return strncmp(bounded.data, string, length) == 0;
}


UNIT(make_http_response_string) {
	SPEC("turns simple responses into strings") {
		HttpResponse response = {
			.version = {
				.major = 1,
				.minor = 0
			},
			.status_code = 200,

			.n_headers = 0,
			
			.content = make_bounded_string("")
		};
		Char* expected =
			"HTTP/1.0 200 OK\n"
			"\n";
		BoundedString response_string;
		if (make_http_response_string(response, &response_string) != 0) LEAVE("could not make response");

		COMPARE(response_string, equ, expected);
		free_bounded_string(response_string);
		
		DONE;
	}
	SPEC("turns complex responses into strings") {
		HttpHeader headers [2] = {
			{
				.name_code = CONTENT_LENGTH,
				.value = make_bounded_string("29")
			},
			{
				.name_code = CONTENT_TYPE,
				.value = make_bounded_string("text/html")

			}
		};
		HttpResponse response = {
			.version = {
				.major = 1,
				.minor = 1
			},
			.status_code = 404,

			.n_headers = 2,
			.headers = headers,

			.content = make_bounded_string("<h1>404</h1>\n<p>Not Found</p>")
		};
		Char* expected =
			"HTTP/1.1 404 Not Found\n"
			"Content-Length: 29\n"
			"Content-Type: text/html\n"
			"\n"
			"<h1>404</h1>\n"
			"<p>Not Found</p>";
		BoundedString response_string;
		if (make_http_response_string(response, &response_string) != 0) LEAVE("could not make response");

		COMPARE(response_string, equ, expected);
		free_bounded_string(response_string);

		DONE;
	}
}

UNIT(free_http_response) {
	SPEC("frees properly") {
		// Why does this test require so much setup
		// It's not even going to do anything
		HttpHeader header1 = {
				.name_code = CONTENT_LENGTH,
			.value = make_dynamic_bounded_string("7")
		};
		HttpHeader header2 = {
				.name_code = CONTENT_TYPE,
			.value = make_dynamic_bounded_string("text/plain")
		};
		HttpHeader* headers = malloc(2*sizeof(HttpHeader));
		headers[0] = header1;
		headers[1] = header2;

		HttpResponse response = {
			.version = {
				.major = 1,
				.minor = 1
			},
			.status_code = 500,

			.n_headers = 2,
			.headers = headers,

			.content = make_dynamic_bounded_string("FREE ME")
		};

		free_http_response(response);
		
		DONE;
	}
}

DRIVER {
	TEST(make_http_response_string);
	TEST(free_http_response);
}