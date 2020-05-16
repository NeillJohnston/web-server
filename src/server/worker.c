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
Turn a streamed string into an HTTP request.
*/
static ErrorCode streamed_to_request(StreamedString* streamed_request_string, HttpRequest* request) {
	BoundedString request_string;

	if (bounded_from_streamed_string(streamed_request_string, &request_string) != 0) return -1;
	if (parse_http_request(request_string, request) != 0) {
		free_bounded_string(request_string);
		return -1;
	}

	free_bounded_string(request_string);

	return 0;
}

/*
Receive an HTTP request over HTTPS.
*/
static ErrorCode recv_ssl_request(SSL* ssl, HttpRequest* request) {
	StreamedString streamed_request_string;

	if (read_ssl_streamed_string(ssl, &streamed_request_string) != 0) return -1;
	if (streamed_to_request(&streamed_request_string, request) != 0) {
		free_streamed_string(&streamed_request_string);
		return -1;
	}

	free_streamed_string(&streamed_request_string);

	return 0;
}

/*
Receive an HTTP request.
*/
static ErrorCode recv_request(Socket connection, HttpRequest* request) {
	StreamedString request_string;

	if (read_streamed_string(connection, &request_string) != 0) return -1;
	if (streamed_to_request(&request_string, request) != 0) {
		free_streamed_string(&request_string);
		return -1;
	}

	free_streamed_string(&request_string);

	return 0;
}

/*
Send an HTTP response over HTTPS.
*/
static ErrorCode send_ssl_response(SSL* ssl, HttpResponse response) {
	BoundedString response_string;

	if (make_http_response_string(response, &response_string)) return -1;
	if (SSL_write(ssl, response_string.data, response_string.length) == -1) {
		free_bounded_string(response_string);
		return -1;
	}

	free_bounded_string(response_string);

	return 0;
}

/*
Send an HTTP response.
*/
static ErrorCode send_response(Socket connection, HttpResponse response) {
	BoundedString response_string;

	if (make_http_response_string(response, &response_string) != 0) return -1;
	if (write(connection, response_string.data, response_string.length) == -1) {
		free_bounded_string(response_string);
		return -1;
	}

	free_bounded_string(response_string);

	return 0;
}

/*
Perform the task of receiving requests and sending responses.
*/
static Void be_worker(SSL* ssl, ServerConfig config) {
	HttpRequest request;

	// TODO: honor Keep-Alive, maybe some other headers
	if (recv_ssl_request(ssl, &request) != 0) {
		send_ssl_response(ssl, DEFAULT_RESPONSE);
		return;
	}

	printf("%s %.*s\n",
		METHOD_NAMES[request.method_code],
		(Int) request.request_uri.uri.length, request.request_uri.uri.data
	);

	HttpResponse response = respond(request, config);
	send_ssl_response(ssl, response);
	
	SSL_shutdown(ssl);

	free_http_request(request);
	free_http_response(response);
	SSL_free(ssl);
}

/*
Perform the task of receiving requests and sending responses, for the dev server.
*/
static Void be_dev_worker(Socket connection, ServerConfig config) {
	HttpRequest request;

	if (recv_request(connection, &request) != 0) {
		send_response(connection, DEFAULT_RESPONSE);
		return;
	}

	printf("%s %.*s\n",
		METHOD_NAMES[request.method_code],
		(Int) request.request_uri.uri.length, request.request_uri.uri.data
	);

	HttpResponse response = respond(request, config);
	send_response(connection, response);

	close(connection);

	free_http_request(request);
	free_http_response(response);
}

ErrorCode spawn_worker(SSL* ssl, ServerConfig config, Pid* worker_pid) {
	Pid forked = fork();
	
	if (forked == (Pid) -1) return -1;
	else if (forked == 0) {
		be_worker(ssl, config);
		_exit(0);
	}
	else {
		*worker_pid = forked;
	}

	return 0;
}

ErrorCode spawn_dev_worker(Socket connection, ServerConfig config, Pid* worker_pid) {
	Pid forked = fork();

	if (forked == (Pid) -1) return -1;
	else if (forked == 0) {
		be_dev_worker(connection, config);
		_exit(0);
	}
	else {
		*worker_pid = forked;
	}

	return 0;
}