#include "site.h"

#include <fcntl.h>

static const BoundedString INDEX_HTML = {
	.data = "/index.html\0",
	.length = 12
};

UInt route_site(BoundedString root, BoundedString route, HttpResponse* response) {
	Char path_data [1024];
	BoundedString path = {
		.data = path_data,
		.length = 0
	};

	append_inplace(root, &path);
	append_inplace(route, &path);
	append_inplace(INDEX_HTML, &path);

	FileDescriptor index_file = open(path.data, O_RDONLY);
	if (index_file == -1) return 404;

	StreamedString streamed_content;
	if (read_streamed_string(index_file, &streamed_content) != 0) return 500;

	BoundedString content;
	if (bounded_from_streamed_string(&streamed_content, &content) != 0) return 500;

	response->content = content;

	return 200;
}