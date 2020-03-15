#include "site.h"
#include "../../files/files.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>

static const BoundedString INDEX_HTML = {
	.data = "/index.html\0",
	.length = 12
};

UInt route_site(BoundedString root, BoundedString path, HttpResponse* response) {
	Char path_data [PATH_MAX];
	BoundedString full_path = {
		.data = path_data,
		.length = 0
	};

	// Check for suspicious paths
	// TODO: find out if there's any other way to abuse paths and check accordingly
	for (Size i = 0; i < path.length; ++i)
		if (path.data[i] == '.' && path.data[i+1] == '.') return 403;

	append_inplace(root, &full_path);
	append_inplace(path, &full_path);
	append_inplace(INDEX_HTML, &full_path);

	BoundedString contents;
	if (get_file_contents(full_path, &contents) != 0) {
		if (errno == ENOENT) return 404;
		else return 500;
	}

	response->content = contents;

	return 200;
}