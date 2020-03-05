#pragma once

#include <common_types.h>
#include "streamed_string.h"

/*
A string (Char array) with a given length.

I really don't want to deal with C-strings.
*/
typedef struct {
	Char* data;
	Size length;
} BoundedString;

/*
Make a BoundedString from a StreamedString.
Writes back to bounded.
The data of the resulting BoundedString will be dynamically allocated, but not
the struct itself.
*/

ErrorCode bounded_from_streamed_string(StreamedString* string, BoundedString* bounded);