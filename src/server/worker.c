#include "server.h"
#include "http/http.h"
#include "router/router.h"
#include "string/streamed_string.h"
#include "string/bounded_string.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Default 500 response in case of error
static const HttpResponse DEFAULT_RESPONSE = {
	.version = {
		.major = 1,
		.minor = 1
	},
	.status_code = 500,
	.n_headers = 0,
	.content = {
		.data = "",
		.length = 0
	}
};

static Void send_response(Socket socket, HttpResponse response) {
	BoundedString response_string;
	// TODO: find out how to handle these two errors gracefully
	if (make_http_response_string(response, &response_string)) _exit(-1);
	if (send(socket, response_string.data, response_string.length, 0) == -1) _exit(-1);
}

/*
Perform the task of receiving requests and sending responses.
*/
static Void be_worker(Socket socket, ServerConfig config) {
	// TODO: use the headers given (Connection: keep-alive, etc.)

	StreamedString streamed_request_string;
	BoundedString request_string;
	HttpRequest request;

	if (read_streamed_string(socket, &streamed_request_string) != 0) {
		send_response(socket, DEFAULT_RESPONSE);
		_exit(-1);
	}
	if (bounded_from_streamed_string(&streamed_request_string, &request_string) != 0) {
		send_response(socket, DEFAULT_RESPONSE);
		_exit(-1);
	}
	if (parse_http_request(request_string, &request) != 0) {
		send_response(socket, DEFAULT_RESPONSE);
		_exit(-1);
	}

	HttpResponse response = respond(request, config);
	send_response(socket, response);

	_exit(0);
}

ErrorCode spawn_worker(Socket socket, ServerConfig config, Pid* worker_pid) {
	const Pid INVALID = -1;

	Pid forked = fork();
	
	if (forked == INVALID) return -1;
	else if (forked == 0) {
		be_worker(socket, config);
	}
	else {
		*worker_pid = forked;
	}

	return 0;
}