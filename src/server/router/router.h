#pragma once

#include "../server.h"
#include "../http/http.h"

// TODO: Make const, find a nice way to do this
static Char* STATUS_MESSAGES [600] = {
	[200] = "OK",
	[403] = "Forbidden",
	[404] = "Not found",
	[500] = "Internal error"
};

/*
The server routes HTTP requests to three different sub-routers -
-	Site, which handles requests for top-level routes (e.g. /)
-	Static, which serves static content (e.g. /content/image.png)
-	Dynamic, which connects to database/API content (e.g. /api/login)

The respond function below decides which sub-router to forward the request to.
*/

/*
Respond to a request.
Does not error - any errors must be HTTP errors in the response.
*/
HttpResponse respond(HttpRequest request, ServerConfig config);