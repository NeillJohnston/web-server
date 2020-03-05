#pragma once

#include "bounded_string.h"

#include <common_types.h>
#include <unistd.h>
#include <limits.h>

// TODO: NODE_SIZE is set to the virtual page size (on dev computer)
// 1. Find out if this can be determined at compile-time, for other machines
// 2. Find out if this is actually helps memory
#define NODE_SIZE (0x1000)

/*
Represents a (node of a) potentially infinitely-long string that gets read
from a pipe/socket/etc.
String gets read in small blocks, which can then be coalesced into a single
string. In typical linked-list fashion, a NULL value for next indicates the
end of the string.
*/
typedef struct StreamedStringNode {
	Char data [NODE_SIZE - sizeof(struct StreamedStringNode*) - sizeof(Size)];
	struct StreamedStringNode* next;
	Size n_bytes;
} StreamedStringNode;

/*
Represents a potentially infinitely-long string that gets read from a pipe/
socket/etc.

IMPORTANT: Any function that uses StreamedStrings should be very aware of the
memory layout. The string itself holds the head's data instead of a pointer to
(hopefully) improve performance.

Most requests should be less than NODE_SIZE, so ideally
multiple nodes won't have to be used. This means that the majority of strings
can be statically allocated - so just create the space needed on the stack and
dynamically allocate subsequent nodes as needed.

One caveat, though, is that any method that takes a StreamedString as a param
should probably take a pointer to it instead, as the SreamedString type itself
can be quite large. (Probably not unreasonably large for a modern computer,
but just a precaution).
*/
typedef struct {
	StreamedStringNode head;
} StreamedString;

/*
Read in a string from a stream, returning an error if necessary.
Does not cap the size of the resulting string.
Writes back to the given string.
*/
ErrorCode read_streamed_string(FileDescriptor stream, StreamedString* string);

/*
Free the memory allocated to a streamed string.
It is assumed that the head node has not been dynamically allocated.
*/
Void free_streamed_string(StreamedString* string);

/*
Make a BoundedString from a StreamedString.
The data of the resulting BoundedString will be dynamically allocated, but not
the struct itself.
*/
BoundedString streamed_to_bounded_string(StreamedString* string);