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

UNIT(add_http_header) {
	SPEC("adds headers to response") {
		enum HeaderNameCode name_code1 = CONTENT_TYPE;
		BoundedString value1 = make_bounded_string("text/html");
		enum HeaderNameCode name_code2 = CONTENT_LENGTH;
		BoundedString value2 = make_bounded_string("100");
		HttpResponse response = {
			.version = {
				.major = 1,
				.minor = 0
			},
			.status_code = 200,

			.n_headers = 0,
			.headers = NULL,
	
			.content = make_bounded_string("")
		};

		if (add_http_header(name_code1, value1, &response) != 0) LEAVE("could not add header");

		ASSERT(response.n_headers == 1);
		ASSERT(response.headers[0].name_code == name_code1);
		COMPARE(response.headers[0].value, bounded_string_equ, value1);

		if (add_http_header(name_code2, value2, &response) != 0) LEAVE("could not add header");
		
		ASSERT(response.n_headers == 2);
		ASSERT(response.headers[1].name_code == name_code2);
		COMPARE(response.headers[1].value, bounded_string_equ, value2);
		// Make sure we don't modify the old data on accident
		ASSERT(response.headers[0].name_code == name_code1);
		COMPARE(response.headers[0].value, bounded_string_equ, value1);

		DONE;
	}
}

UNIT(add_blank_http_header) {
	SPEC("adds blank headers to a response") {
		HttpResponse response = {
			.n_headers = 0,
			.headers = NULL
		};

		if (add_blank_http_header(CONTENT_TYPE, &response) != 0) LEAVE("could not add header");

		ASSERT(response.n_headers == 1);
		ASSERT(response.headers[0].name_code == CONTENT_TYPE);

		DONE;
	}
}

UNIT(get_http_response_header) {
	HttpHeader response_headers [2] = {
		{
			.name_code = CONTENT_LENGTH,
			.value = make_bounded_string("10")
		},
		{
			.name_code = CONTENT_TYPE,
			.value = make_bounded_string("text")
		}
	};
	HttpResponse response = {
		.n_headers = 2,
		.headers = response_headers
	};

	SPEC("gets existing headers") {
		HttpHeader* content_length;
		HttpHeader* content_type;

		OKAY(get_http_response_header(CONTENT_LENGTH, response, &content_length));
		OKAY(get_http_response_header(CONTENT_TYPE, response, &content_type));
		ASSERT(content_length->value.length == 2);
		ASSERT(content_type->value.length == 4);

		DONE;
	}
	SPEC("errors on nonexistent headers") {
		HttpHeader* accept;

		ERROR(get_http_response_header(ACCEPT, response, &accept));

		DONE;
	}
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
	TEST(add_http_header);
	TEST(add_blank_http_header);
	TEST(get_http_response_header);
	TEST(make_http_response_string);
	TEST(free_http_response);
}