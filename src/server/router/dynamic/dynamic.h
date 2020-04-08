#pragma once

#include "../../http/http.h"
#include "../../string/bounded_string.h"

#include <sqlite3.h>

/*
Get the full results of a query as a JSON-encoded string.
The string will be dynamically allocated, and therefore must be freed.
*/
BoundedString get_json(sqlite3_stmt* query);

/*
Stub for now. Returns 404 unconditionally.
*/
UInt route_dynamic(BoundedString root, BoundedString path, HttpResponse* response);