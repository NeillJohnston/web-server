#include "../router.c"

#include <string.h>
#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

typedef ULong Flags;

const Flags ROUTE_SITE = 1<<0;
const Flags ROUTE_STATIC = 1<<1;
const Flags ROUTE_DYNAMIC = 1<<2;
Flags routed_where;

UInt route_site(BoundedString root, BoundedString route, HttpResponse* response) {
	routed_where |= ROUTE_SITE;
	return 200;
}

UInt route_static(BoundedString root, BoundedString route, HttpResponse* response) {
	routed_where |= ROUTE_STATIC;
	return 200;
}

UInt route_dynamic(BoundedString root, BoundedString route, HttpResponse* response) {
	routed_where |= ROUTE_DYNAMIC;
	return 200;
}

UNIT(respond) {
	SPEC("correctly routes to site paths") {
		HttpRequest request = {
			.method = make_bounded_string("GET"),
			.path = make_bounded_string("/blog"),
			.version = {
				.major = 1,
				.minor = 1
			},
			.n_headers = 0,
			.content = make_bounded_string("")
		};
		ServerConfig config = {
			.root = make_bounded_string("src/server/router/_/root/"),
			.api_prefix = make_bounded_string("/api/")
		};

		routed_where = 0;
		HttpResponse response = respond(request, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == ROUTE_SITE);

		DONE;
	}
	SPEC("correctly routes to static paths") {
		HttpRequest request = {
			.method = make_bounded_string("GET"),
			.path = make_bounded_string("/content/image.jpeg"),
			.version = {
				.major = 1,
				.minor = 1
			},
			.n_headers = 0,
			.content = make_bounded_string("")
		};
		ServerConfig config = {
			.root = make_bounded_string("src/server/router/_/root/"),
			.api_prefix = make_bounded_string("/api/")
		};

		routed_where = 0;
		HttpResponse response = respond(request, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == ROUTE_STATIC);

		DONE;
	}
	SPEC("correctly routes to dynamic (api) paths") {
		HttpRequest request = {
			.method = make_bounded_string("GET"),
			.path = make_bounded_string("/api/get-something"),
			.version = {
				.major = 1,
				.minor = 1
			},
			.n_headers = 0,
			.content = make_bounded_string("")
		};
		ServerConfig config = {
			.root = make_bounded_string("src/server/router/_/root/"),
			.api_prefix = make_bounded_string("/api/")
		};

		routed_where = 0;
		HttpResponse response = respond(request, config);
		ASSERT(response.status_code == 200);
		ASSERT(routed_where == ROUTE_DYNAMIC);

		DONE;
	}
}

DRIVER {
	TEST(respond);
}