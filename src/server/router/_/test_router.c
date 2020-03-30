#include "../router.c"
#include "../../string/bounded_string.c"
#include "../../http/http_response.c"

#include <string.h>
#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

HttpRequest make_request(Char* uri, enum MethodCode method_code) {
	return (HttpRequest) {
		.method_code = method_code,
		.request_uri = {
			.type = ABS_PATH,
			.uri = make_bounded_string(uri)
		},
		.version = {
			.major = 1,
			.minor = 1
		},
		.n_headers = 0
	};
}

typedef ULong Flags;

const Flags FLAG_STATIC = 1<<1;
const Flags FLAG_DYNAMIC = 1<<2;
Flags routed_where;

UInt route_static(BoundedString root, BoundedString path, HttpResponse* response) {
	routed_where |= FLAG_STATIC;
	// Triggered by the error reroute testcase
	if (path.data[0] == '*') return 404;
	return 200;
}

UInt route_dynamic(BoundedString root, BoundedString path, HttpResponse* response) {
	routed_where |= FLAG_DYNAMIC;
	// Should be triggered by the error reroute testcase
	if (path.data[0] == '*') {
		response->content.length = 404;
		return 200;
	}
	return 200;
}

UNIT(respond) {
	SPEC("looks up the correct route in database") {
		// Make sure the route and content matches test.db or obviously the test will fail
		HttpRequest request1 = make_request("/", GET);
		HttpRequest request2 = make_request("/blog", GET);
		HttpRequest request3 = make_request("/api/post", POST);
		ServerConfig config = {
			.db_path = make_bounded_string("src/server/router/_/test.db")
		};
		HttpResponse response;

		routed_where = 0;
		response = respond(request1, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == FLAG_STATIC);

		routed_where = 0;
		response = respond(request2, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == FLAG_STATIC);

		routed_where = 0;
		response = respond(request3, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == FLAG_DYNAMIC);

		DONE;
	}
	SPEC("routes unknown routes to static") {
		HttpRequest request1 = make_request("/style.css", GET);
		ServerConfig config = {
			.db_path = make_bounded_string("src/server/router/_/test.db")
		};
		HttpResponse response;

		routed_where = 0;
		response = respond(request1, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == FLAG_STATIC);

		DONE;
	}
	SPEC("re-routes to special error routes") {
		HttpRequest request1 = make_request("*nonexistent", GET);
		ServerConfig config = {
			.db_path = make_bounded_string("src/server/router/_/test.db")
		};
		HttpResponse response;

		routed_where = 0;
		response = respond(request1, config);
		ASSERT(response.status_code == 404);
		ASSERT(response.content.length == 404);
		ASSERT(routed_where == (FLAG_STATIC | FLAG_DYNAMIC));

		DONE;
	}
}

DRIVER {
	TEST(respond);
}