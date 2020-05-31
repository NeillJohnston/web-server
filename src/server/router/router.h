#pragma once

#include "../server.h"
#include "../http/http.h"

enum RouteType {
	ROUTE_STATIC = 0,
	ROUTE_DYNAMIC = 1
};

/*
Respond to a request.
Can not error - any "errors" must be HTTP errors in the response.
*/
HttpResponse respond(HttpRequest request, ServerConfig config);