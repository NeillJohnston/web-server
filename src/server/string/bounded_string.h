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
Compare a bounded string x to a C-string y, return whether they are the same string.
*/
Bool bounded_string_equ_cstr(BoundedString x, const Char* y);

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
"Pop" a delimited token (characters up to the first instance of delimiter)
from a bounded string, by modifying the length of string to not include the
popped token or the instance of delimiter.

Similar to the other pop- functions.

When the last delimited token is popped, string will have a length of 0.
*/
BoundedString pop_delimited_inplace(BoundedString* string, Char delimiter);

/*
Append suffix to string, inplace (without making a new bounded string).
Caller must do necssary memory management and ensure that the data buffer of
string is large enough to hold the suffix.
*/
Void append_inplace(BoundedString suffix, BoundedString* string);

/*
Append a C-style string suffic to string, inplace (without making a new bounded string).
Caller must do necssary memory management and ensure that the data buffer of
string is large enough to hold the suffix.
*/
Void append_cstr_inplace(const Char* suffix, BoundedString* string);

/*
Trim whitespace from the front and back of string.
*/
Void trim_inplace(BoundedString* string);

/*
Copy source to destination, dynamically allocating the memory.
Returns -1 if there's an error with malloc.
Writes back to destination.
*/
ErrorCode copy_bounded_string(BoundedString source, BoundedString* destination);

/*
Make a copy of a bounded string, in C-string format.
Returns -1 if there's a malloc error.
Writes back to destination.
*/
ErrorCode copy_bounded_string_to_cstr(BoundedString source, Char** destination);

/*
Free the data in a bounded string.
*/
Void free_bounded_string(BoundedString string);