#include "router.h"
#include "site/site.h"
#include "static/static.h"
#include "dynamic/dynamic.h"

#include <string.h>

/*
Return whether path refers to a dynamic path.
This means the path is prefixed with config's api_prefix.
*/
static Bool is_dynamic_path(BoundedString path, BoundedString api_prefix) {
	if (api_prefix.length == 0) return false;

	// Start both substrings on a non-slash for a fair comparison
	Size i = path.data[0] == '/' ? 1 : 0;
	Size j = api_prefix.data[0] == '/' ? 1 : 0;
	
	if (path.length - i < api_prefix.length - j) return false;

	while (i < path.length && j < api_prefix.length) {
		if (path.data[i] != api_prefix.data[j]) return false;
		++i;
		++j;
	}

	return true;
}

/*
Return whether path refers to a static path.
This means that the last part of the path has an extension.
*/
static Bool is_static_path(BoundedString path) {
	for (Size i = path.length-1; i > 0; --i) {
		if (path.data[i] == '.' && path.data[i-1] != '/') return true;
		if (path.data[i] == '/') return false;
	}
	return false;
}

HttpResponse respond(HttpRequest request, ServerConfig config) {
	HttpResponse response = {
		.version = {
			.major = 1,
			.minor = 1
		},
		.status_code = 0,
		.status_message = {
			.data = "",
			.length = 0
		},
		.n_headers = 0,
		.headers = NULL,
		.content = {
			.data = "",
			.length = 0
		}
	};

	if (is_dynamic_path(request.path, config.api_prefix)) {
		response.status_code = route_dynamic(config.root, request.path, &response);
	}
	else if (is_static_path(request.path)) {
		response.status_code = route_static(config.root, request.path, &response);
	}
	else {
		response.status_code = route_site(config.root, request.path, &response);
	}

	if (STATUS_MESSAGES[response.status_code] == NULL) {
		response.status_code = 500;
	}

	Char* status_message_cstr = STATUS_MESSAGES[response.status_code];
	response.status_message = (BoundedString) {
		.data = status_message_cstr,
		.length = strlen(status_message_cstr)
	};

	return response;
}