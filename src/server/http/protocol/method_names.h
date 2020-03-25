#pragma once

#include "../http.h"

static const Char* METHOD_NAMES [] = {
	[GET] = "GET",
	[POST] = "POST",
	[PUT] = "PUT",
	[DELETE] = "DELETE",
	[OPTIONS] = "OPTIONS",
	[HEAD] = "HEAD",
	[TRACE] = "TRACE",
	[CONNECT] = "CONNECT"
};