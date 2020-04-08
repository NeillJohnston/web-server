#include "static.h"
#include "../../files/files.h"

#include <errno.h>
#include <linux/limits.h>

UInt route_static(BoundedString root, BoundedString path, HttpResponse* response) {
	Char path_data [PATH_MAX];
	BoundedString full_path = {
		.data = path_data,
		.length = 0
	};

	if (root.length + path.length + 1 > PATH_MAX) return 500;
	// Check for invalid paths (paths with a "..")
	for (Size i = 0; i < path.length-1; ++i)
		if (path.data[i] == '.' && path.data[i+1] == '.') return 403;

	append_inplace(root, &full_path);
	append_inplace(path, &full_path);

	ErrorCode attempt_get_contents = get_file_contents(full_path, &response->content);
	if (attempt_get_contents == ERROR_NO_FILE) return 404;
	else if (attempt_get_contents != 0) return 500;

	return 200;
}