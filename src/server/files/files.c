#include "files.h"
#include "../string/streamed_string.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>

ErrorCode get_file_contents(BoundedString path, BoundedString* contents) {
	if (path.length > PATH_MAX-1) return -1;

	Char terminated_path [PATH_MAX];
	if (path.length+1 > PATH_MAX) return -1;
	memcpy(terminated_path, path.data, path.length);
	terminated_path[path.length] = '\0';

	FileDescriptor file = open(terminated_path, O_RDONLY);
	if (file == -1) {
		if (errno == ENOENT || errno == EISDIR) return ERROR_NO_FILE;
		else return -1;
	}

	StreamedString streamed_contents;
	if (read_streamed_string(file, &streamed_contents) != 0) return -1;

	if (bounded_from_streamed_string(&streamed_contents, contents) != 0) return -1;

	return 0;
}