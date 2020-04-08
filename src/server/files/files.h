#pragma once

#include "../string/bounded_string.h"

/*
General "no file" error: treats ENOENT (no file exists) and EISDIR (file is
directory) as the same, check errno if the distinction needs to be made
*/
static const ErrorCode ERROR_NO_FILE = 1;

/*
Get the contents of a file pointed to by path.
Writes back to contents. The data in contents will be dynamically allocated.
*/
ErrorCode get_file_contents(BoundedString path, BoundedString* contents);