#include "static.h"
#include "../../files/files.h"

#include <errno.h>
#include <linux/limits.h>
#include <string.h>

/*
Test if a path has a given extension.
*/
static BoundedString get_extension(BoundedString path) {
	Size i = path.length;

	while (i > 0 && path.data[i] != '.') {
		--i;
	}

	return (BoundedString) {
		.data = path.data + (i+1),
		.length = path.length - (i+1)
	};
}

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

	// Content type header
	HttpHeader* header;
	if (get_http_response_header(CONTENT_TYPE, *response, &header) != 0) return 500;

	Char* content_type_data;
	BoundedString extension = get_extension(path);
	if (     bounded_string_equ_cstr(extension, "html")) content_type_data = "text/html";
	else if (bounded_string_equ_cstr(extension, "css"))  content_type_data = "text/css";
	else if (bounded_string_equ_cstr(extension, "js"))   content_type_data = "text/js";
	else if (bounded_string_equ_cstr(extension, "png"))  content_type_data = "image/png";
	else if (bounded_string_equ_cstr(extension, "jpg"))  content_type_data = "image/jpeg";
	else if (bounded_string_equ_cstr(extension, "jpeg")) content_type_data = "image/jpeg";
	else if (bounded_string_equ_cstr(extension, "pdf"))  content_type_data = "application/pdf";
	else content_type_data = "text/plain";

	BoundedString content_type = { .data = content_type_data, .length = strlen(content_type_data) };
	if (copy_bounded_string(content_type, &header->value) != 0) return 500;

	return 200;
}