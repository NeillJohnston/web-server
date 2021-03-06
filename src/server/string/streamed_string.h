#pragma once

#include <common_types.h>
#include <limits.h>
#include <openssl/ssl.h>
#include <unistd.h>

static const ErrorCode ERROR_INVALIDATED_SSL = 1;

// Must be a multiple of 8 due to struct packing
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
	Size size;
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

If this function errors, then this function will free the memory that has been
created so far before exiting - the caller does not have to.
*/
ErrorCode read_streamed_string(FileDescriptor stream, StreamedString* string);

/*
Read in a string from an encrypted SSL stream, returning an error if necessary.
Does not cap the size of the resulting string.
Writes back to the given string.

If this function errors, then this function will free the memory that has been
created so far before exiting - the caller does not have to.
*/
ErrorCode read_ssl_streamed_string(SSL* ssl, StreamedString* string);

/*
Free the memory allocated to a streamed string.
It is assumed that the head node has not been dynamically allocated.
*/
Void free_streamed_string(StreamedString* string);