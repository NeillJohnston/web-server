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

	// Check for suspicious paths, like in route_static
	for (Size i = 0; i < path.length; ++i)
		if (path.data[i] == '.' && path.data[i+1] == '.') return 403;

	append_inplace(root, &full_path);
	append_inplace(path, &full_path);

	if (get_file_contents(full_path, &response->content) != 0) {
		if (errno == ENOENT) return 404;
		else return 500;
	}

	return 200;
}