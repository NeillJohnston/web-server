#pragma once

#include <common_types.h>

/*
A string (Char array) with a given length.

I really don't want to deal with C-strings.
*/
typedef struct {
	Char* data;
	Size length;
} BoundedString;