#pragma once

#include "../http.h"

static const Char* HEADER_NAMES [] = {
	[ACCEPT] = "Accept",
	[CONNECTION] = "Connection",
	[CONTENT_LENGTH] = "Content-Length",
	[CONTENT_TYPE] = "Content-Type",
	[HOST] = "Host",
	[USER_AGENT] = "User-Agent"
};