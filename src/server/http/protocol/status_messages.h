#pragma once

#include "../http.h"

static const Char* STATUS_MESSAGES [] = {
	[200] = "OK",
	[403] = "Forbidden",
	[404] = "Not Found",
	[500] = "Internal Error"
};