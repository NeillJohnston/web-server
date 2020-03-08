#include "../http_request.c"

#include "../../string/bounded_string.c"

#include <fcntl.h>
#include <string.h>
#include <underscore.h>

Char* SIMPLE =
"\
GET /index.html HTTP/1.0\n\
\n\
";

Char* HEADERS =
"\
GET / HTTP/1.1\n\
Host: 10.20.0.210:3000\n\
Connection: keep-alive\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.122 Safari/537.36 Edg/80.0.361.62\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\n\
\n\
";

Char* CONTENT =
"\
POST /post HTTP/1.1\n\
Host: localhost\n\
Content-Type: key-value\n\
Content-Length: 7\n\
\n\
a:1\n\
b:2\
";

Char* INVALID1 =
"\
GET / HTTP1.1\n\
\n\
";

Char* INVALID2 =
"\
GET / HTTP/1.1\n\
Header without colon\n\
\n\
";

Char* INVALID3 =
"\
GET / HTTP/1.1\n\
Error: no newlines for content\n\
";

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

UNIT(parse_http_request) {
	SPEC("correctly parses very simple requests") {
		BoundedString request_string = make_bounded_string(SIMPLE);
		HttpRequest request;

		OKAY(parse_http_request(request_string, &request));
		COMPARE(request.method, equ, "GET");
		COMPARE(request.path, equ, "/index.html");
		ASSERT(request.version.major == 1);
		ASSERT(request.version.minor == 0);
		ASSERT(request.n_headers == 0);
		COMPARE(request.content, equ, "");
		COMPARE(request.raw, equ, SIMPLE);
		
		DONE;
	}
	SPEC("correctly parses requests with headers") {
		BoundedString request_string = make_bounded_string(HEADERS);
		HttpRequest request;

		OKAY(parse_http_request(request_string, &request));
		COMPARE(request.method, equ, "GET");
		COMPARE(request.path, equ, "/");
		ASSERT(request.version.major == 1);
		ASSERT(request.version.minor == 1);
		ASSERT(request.n_headers == 4);
		COMPARE(request.headers[0].name, equ, "Host");
		COMPARE(request.headers[0].value, equ, "10.20.0.210:3000");
		COMPARE(request.headers[1].name, equ, "Connection");
		COMPARE(request.headers[1].value, equ, "keep-alive");
		COMPARE(request.headers[2].name, equ, "User-Agent");
		COMPARE(request.headers[2].value, equ, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.122 Safari/537.36 Edg/80.0.361.62");
		COMPARE(request.headers[3].name, equ, "Accept");
		COMPARE(request.headers[3].value, equ, "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
		COMPARE(request.content, equ, "");
		COMPARE(request.raw, equ, HEADERS);

		DONE;
	}
	SPEC("correctly parses requests with headers and content") {
		BoundedString request_string = make_bounded_string(CONTENT);
		HttpRequest request;

		OKAY(parse_http_request(request_string, &request));
		COMPARE(request.method, equ, "POST");
		COMPARE(request.path, equ, "/post");
		ASSERT(request.version.major == 1);
		ASSERT(request.version.minor == 1);
		ASSERT(request.n_headers == 3);
		COMPARE(request.headers[0].name, equ, "Host");
		COMPARE(request.headers[0].value, equ, "localhost");
		COMPARE(request.headers[1].name, equ, "Content-Type");
		COMPARE(request.headers[1].value, equ, "key-value");
		COMPARE(request.headers[2].name, equ, "Content-Length");
		COMPARE(request.headers[2].value, equ, "7");
		COMPARE(request.content, equ, "a:1\nb:2");
		COMPARE(request.raw, equ, CONTENT);

		DONE;
	}
	SPEC("errors for invalid HTTP requests") {
		BoundedString invalid1 = make_bounded_string(INVALID1);
		BoundedString invalid2 = make_bounded_string(INVALID2);
		BoundedString invalid3 = make_bounded_string(INVALID3);
		HttpRequest temp;

		ERROR(parse_http_request(invalid1, &temp));
		ERROR(parse_http_request(invalid2, &temp));
		ERROR(parse_http_request(invalid3, &temp));

		DONE;
	}
}

UNIT(free_http_request) {
	SPEC("frees properly") {
		BoundedString simple_string = make_bounded_string(SIMPLE);
		BoundedString headers_string = make_bounded_string(HEADERS);
		BoundedString content_string = make_bounded_string(CONTENT);
		HttpRequest request;

		if (parse_http_request(simple_string, &request) != 0) LEAVE("could not parse simple");
		free_http_request(request);
		
		if (parse_http_request(headers_string, &request) != 0) LEAVE("could not parse headers");
		free_http_request(request);
		
		if (parse_http_request(content_string, &request) != 0) LEAVE("could not parse content");
		free_http_request(request);
		
		// No real "tests", just make sure free doesn't fail
		DONE;
	}
}

DRIVER {
	TEST(parse_http_request);
	TEST(free_http_request);
}