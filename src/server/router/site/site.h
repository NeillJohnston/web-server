#pragma once

#include "../router.h"

/*
A site route is a top-level route, and should basically look like a path to a
directory (instead of a file).

For example, "/", "/blog", and "/about" are site routes.

The router itself will serve the content of index.html in the requested
directory.
*/

/*
Route to a site route, returning the HTTP status code.
Writes back content and any necessary headers to response.
*/
UInt route_site(BoundedString root, BoundedString route, HttpResponse* response);