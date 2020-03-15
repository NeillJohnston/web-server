#pragma once

#include "../../http/http.h"
#include "../../string/bounded_string.h"

/*
Writes the content of the file at root+path to response.content.
Returns an HTTP status code indicating how the operation went.
*/
UInt route_static(BoundedString root, BoundedString path, HttpResponse* response);