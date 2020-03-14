#pragma once

#include "../string/bounded_string.h"

/*
Get the contents of a file pointed to by path.
Writes back to contents. The data in contents will be dynamically allocated.
*/
ErrorCode get_file_contents(BoundedString path, BoundedString* contents);