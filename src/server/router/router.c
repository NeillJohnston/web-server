#include "router.h"
#include "site/site.h"

HttpResponse respond(HttpRequest request, ServerConfig config) {
	HttpResponse response;

	response.version = (HttpVersion) {
		.major = 1,
		.minor = 1
	};
	response.status_code = route_site(config.root, request.path, &response);
	// TODO: support the other sub-routers
	response.status_message = (BoundedString) {
		.data = "OK",
		.length = 2
	};
	response.n_headers = 0;

	return response;
}