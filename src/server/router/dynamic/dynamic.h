#pragma once

#include "../../http/http.h"
#include "../../string/bounded_string.h"

/*
Stub for now. Returns 404 unconditionally.
*/
UInt route_dynamic(BoundedString root, BoundedString path, HttpResponse* response);