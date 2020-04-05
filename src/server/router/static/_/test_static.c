#include "../static.c"
#include "../../../files/files.c"
#include "../../../string/bounded_string.c"
#include "../../../string/streamed_string.c"

#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

Bool equ(BoundedString bounded, Char* cstr) {
	if (bounded.length != strlen(cstr)) return false;
	return memcmp(bounded.data, cstr, bounded.length) == 0;
}

UNIT(route_static) {
	SPEC("gets the content of files in the root directory") {
		BoundedString root = make_bounded_string("src/server/router/static/_/root/");
		BoundedString path = make_bounded_string("content.txt");
		HttpResponse response;

		ASSERT(route_static(root, path, &response) == 200);
		COMPARE(response.content, equ, "content");

		DONE;
	}
	SPEC("gets the content of files in a sub directory") {
		BoundedString root = make_bounded_string("src/server/router/static/_/root/");
		BoundedString path = make_bounded_string("sub/image.jpeg");
		HttpResponse response;

		ASSERT(route_static(root, path, &response) == 200);
		COMPARE(response.content, equ, "do i look like i know what a jpeg is");

		DONE;
	}
	SPEC("404s on non-existing files") {
		BoundedString root = make_bounded_string("src/server/router/static/_/root/");
		BoundedString path = make_bounded_string("no");
		HttpResponse response;

		ASSERT(route_static(root, path, &response) == 404);

		DONE;
	}
	SPEC("403s on suspicious paths") {
		BoundedString root = make_bounded_string("src/server/router/static/_/root/");
		BoundedString path = make_bounded_string("../../secrets/passwords.txt");
		HttpResponse response;

		ASSERT(route_static(root, path, &response) == 403);

		DONE;
	}
}

DRIVER {
	TEST(route_static);
}

// Blank stubs
Int SSL_read(SSL* ssl, Void* buffer, Int num) {
	return 0;
}