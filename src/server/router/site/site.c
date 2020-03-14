#include "site.h"
#include "../../files/files.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>

static const BoundedString INDEX_HTML = {
	.data = "/index.html\0",
	.length = 12
};

UInt route_site(BoundedString root, BoundedString route, HttpResponse* response) {
	Char path_data [PATH_MAX];
	BoundedString path = {
		.data = path_data,
		.length = 0
	};

	// Check for suspicious paths
	// TODO: find out if there's any other way to abuse paths and check accordingly
	for (Size i = 0; i < route.length; ++i)
		if (route.data[i] == '.' && route.data[i+1] == '.') return 403;

	append_inplace(root, &path);
	append_inplace(route, &path);
	append_inplace(INDEX_HTML, &path);

	BoundedString contents;
	if (get_file_contents(path, &contents) != 0) {
		if (errno == ENOENT) return 404;
		else return 500;
	}

	response->content = contents;

	return 200;
}