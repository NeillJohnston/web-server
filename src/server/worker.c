#include "server.h"
#include "http/http.h"
#include "http/protocol/method_names.h"
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

/*
Send a response over SSL.
*/
static Void send_response(SSL* ssl, HttpResponse response) {
	BoundedString response_string;
	// TODO: find out how to handle these two errors gracefully
	if (make_http_response_string(response, &response_string)) _exit(-1);
	if (SSL_write(ssl, response_string.data, response_string.length) == -1) _exit(-1);

	free_bounded_string(response_string);
}

/*
Send a response over a socket.
*/
static Void send_dev_response(Socket connection, HttpResponse response) {
	BoundedString response_string;
	// TODO: find out how to handle these two errors gracefully
	if (make_http_response_string(response, &response_string)) _exit(-1);
	if (write(connection, response_string.data, response_string.length) == -1) _exit(-1);

	free_bounded_string(response_string);
}

/*
Perform the task of receiving requests and sending responses.
*/
static Void be_worker(SSL* ssl, ServerConfig config) {
	// TODO: honor HTTP headers
	
	StreamedString streamed_request_string;
	BoundedString request_string;
	HttpRequest request;

	if (read_ssl_streamed_string(ssl, &streamed_request_string) != 0) {
		send_response(ssl, DEFAULT_RESPONSE);
		_exit(-1);
	}
	if (bounded_from_streamed_string(&streamed_request_string, &request_string) != 0) {
		send_response(ssl, DEFAULT_RESPONSE);
		_exit(-1);
	}
	if (parse_http_request(request_string, &request) != 0) {
		send_response(ssl, DEFAULT_RESPONSE);
		_exit(-1);
	}

	printf("%s %.*s\n", METHOD_NAMES[request.method_code], (int) request.request_uri.uri.length, request.request_uri.uri.data);

	HttpResponse response = respond(request, config);
	send_response(ssl, response);
	
	SSL_shutdown(ssl);

	free_streamed_string(&streamed_request_string);
	free_bounded_string(request_string);
	free_http_request(request);
	free_http_response(response);
	SSL_free(ssl);

	_exit(0);
}

/*
Perform the task of receiving requests and sending responses, for the dev server.
*/
static Void be_dev_worker(Socket connection, ServerConfig config) {
	// MARK: for refactor, most of this logic can be offloaded to another function
	
	StreamedString streamed_request_string;
	BoundedString request_string;
	HttpRequest request;

	if (read_streamed_string(connection, &streamed_request_string) != 0) {
		send_dev_response(connection, DEFAULT_RESPONSE);
		_exit(-1);
	}
	if (bounded_from_streamed_string(&streamed_request_string, &request_string) != 0) {
		send_dev_response(connection, DEFAULT_RESPONSE);
		_exit(-1);
	}
	if (parse_http_request(request_string, &request) != 0) {
		send_dev_response(connection, DEFAULT_RESPONSE);
		_exit(-1);
	}

	printf("dev: %s %.*s\n", METHOD_NAMES[request.method_code], (int) request.request_uri.uri.length, request.request_uri.uri.data);

	HttpResponse response = respond(request, config);
	send_dev_response(connection, response);

	shutdown(connection, SHUT_RDWR);

	free_streamed_string(&streamed_request_string);
	free_bounded_string(request_string);
	free_http_request(request);
	free_http_response(response);

	_exit(0);
}

ErrorCode spawn_worker(SSL* ssl, ServerConfig config, Pid* worker_pid) {
	const Pid INVALID = -1;

	Pid forked = fork();
	
	if (forked == INVALID) return -1;
	else if (forked == 0) {
		be_worker(ssl, config);
	}
	else {
		*worker_pid = forked;
	}

	return 0;
}

ErrorCode spawn_dev_worker(Socket connection, ServerConfig config, Pid* worker_pid) {
	const Pid INVALID = -1;

	Pid forked = fork();

	if (forked == INVALID) return -1;
	else if (forked == 0) {
		be_dev_worker(connection, config);
	}
	else {
		*worker_pid = forked;
	}

	return 0;
}