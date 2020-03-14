#include "../site.c"
#include "../../../string/bounded_string.c"
#include "../../../string/streamed_string.c"

#include <string.h>
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

UNIT(route_site) {
	SPEC("serves content from the root") {
		BoundedString root = make_bounded_string("src/server/router/site/_/root/");
		BoundedString route = make_bounded_string("/");
		HttpResponse response;
		UInt status_code = route_site(root, route, &response);

		ASSERT(status_code == 200);
		COMPARE(response.content, equ, "root content");

		DONE;
	}
	SPEC("serves content from a subdirectory") {
		BoundedString root = make_bounded_string("src/server/router/site/_/root/");
		BoundedString route = make_bounded_string("/sub");
		HttpResponse response;
		UInt status_code = route_site(root, route, &response);

		ASSERT(status_code == 200);
		COMPARE(response.content, equ, "sub content");

		DONE;
	}
	SPEC("serves content from a subdirectory (despite having different slashes in the root/route)") {
		BoundedString root = make_bounded_string("src/server/router/site/_/root");
		BoundedString route = make_bounded_string("/sub/");
		HttpResponse response;
		UInt status_code = route_site(root, route, &response);

		ASSERT(status_code == 200);
		COMPARE(response.content, equ, "sub content");

		DONE;
	}
	SPEC("404s for non-existing routes") {
		BoundedString root = make_bounded_string("src/server/router/site/_/root/");
		BoundedString route = make_bounded_string("/nonexistent");
		HttpResponse response;
		UInt status_code = route_site(root, route, &response);

		ASSERT(status_code == 404);

		DONE;
	}
	// TODO: more tests related to security
}

DRIVER {
	TEST(route_site);
}