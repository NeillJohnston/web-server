#pragma once

#include "streamed_string.h"

#include <common_types.h>

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

/*
Compare two bounded strings x and y, return true if their data matches.
*/
Bool bounded_string_equ(BoundedString x, BoundedString y);

/*
"Pop" a line (characters up to the first newline) from a bounded string, by
modifying the original string to not include the line or newline.
"Inplace" refers to the fact that no data is copied, the returned line exists
within the data of the original string.

When the last line is popped, the original string will have a length of 0.

Supports LF and CRLF line endings.
*/
BoundedString pop_line_inplace(BoundedString* string);

/*
"Pop" a token (characters up to the first whitespace) from a bounded string,
by modifying the original string to not include the token or following
whitespace.

Very similar to pop_line_inplace but separates by whitespace instead of by
newlines.

When the last token is popped, the original string will have a length of 0.
*/
BoundedString pop_token_inplace(BoundedString* string);

/*
Copy source to destination, dynamically allocating the memory.
Returns -1 if there's an error with malloc.
Writes back to destination.
*/
ErrorCode copy_bounded_string(BoundedString source, BoundedString* destination);

/*
Free the data in a bounded string.
*/
Void free_bounded_string(BoundedString string);