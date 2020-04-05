#include "server.h"
#include "http/http.h"

#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

ErrorCode init_server(ServerConfig config, InternetServer* server) {
	typedef struct sockaddr SocketAddress;

	// Validate config
	if (config.backlog <= 0) return ERROR_INVALID_OPTIONS;

	// Server socket initialization
	server->address = (InternetSocketAddress) {
		.sin_family = AF_INET,
		.sin_port = htons(config.port),
		.sin_addr.s_addr = htonl(config.local ? INADDR_LOOPBACK : INADDR_ANY)
	};
	server->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server->socket == -1) return ERROR_COULD_NOT_ESTABLISH;

	ErrorCode attempt_bind = bind(server->socket, (SocketAddress*) &server->address, sizeof server->address);
	if (attempt_bind != 0) return ERROR_COULD_NOT_BIND;

	ErrorCode attempt_listen = listen(server->socket, config.backlog);
	if (attempt_listen != 0) return ERROR_COULD_NOT_LISTEN;

	// SSL context initialization
	server->context = SSL_CTX_new(TLS_server_method());
	if (server->context == NULL) return ERROR_SSL_CTX;

	Char cert_path [PATH_MAX];
	memcpy(cert_path, config.cert_path.data, config.cert_path.length);
	cert_path[config.cert_path.length] = '\0';
	if (SSL_CTX_use_certificate_chain_file(server->context, cert_path) != 1) return ERROR_SSL_CTX;

	Char pkey_path [PATH_MAX];
	memcpy(pkey_path, config.pkey_path.data, config.pkey_path.length);
	pkey_path[config.pkey_path.length] = '\0';
	if (SSL_CTX_use_PrivateKey_file(server->context, pkey_path, SSL_FILETYPE_PEM) != 1) return ERROR_SSL_CTX;

	if (SSL_CTX_check_private_key(server->context) != 1) return ERROR_SSL_CTX;

	SSL_CTX_set_mode(server->context, SSL_MODE_AUTO_RETRY);

	return 0;
}

ErrorCode run_redirect_server(ServerConfig config, InternetServer server, Pid* redirect_pid) {
	Pid child = fork();
	if (child == -1) return -1;
	else if (child != 0) {
		*redirect_pid = child;
		return 0;
	}

	while (true) {
		typedef struct sockaddr SocketAddress;
		typedef socklen_t SocketLength;

		SocketAddress incoming;
		SocketLength incoming_length = sizeof incoming;

		Socket connection = accept(server.socket, &incoming, &incoming_length);
		if (connection == -1) continue;

		// Get the request URI
		StreamedString streamed_request_string;
		if (read_streamed_string(connection, &streamed_request_string) != 0) {
			shutdown(connection, SHUT_RDWR);
			continue;
		}

		BoundedString request_string;
		if (bounded_from_streamed_string(&streamed_request_string, &request_string) != 0) {
			shutdown(connection, SHUT_RDWR);
			continue;
		}
		
		HttpRequest request;
		if (parse_http_request(request_string, &request) != 0) {
			shutdown(connection, SHUT_RDWR);
			continue;
		}

		Char message [1024];
		BoundedString uri = request.request_uri.uri;
		printf("redirecting to HTTPS, URI %.*s\n", (Int) uri.length, uri.data);
		Size message_length = (Size) sprintf(
			message,
			"HTTP/1.1 301 Moved Permanently\r\nLocation: https://%.*s%.*s\r\n\r\n",
			(Int) config.domain.length, config.domain.data,
			(Int) uri.length, uri.data
		);
		write(connection, message, message_length);

		shutdown(connection, SHUT_RDWR);
	}
}

Void run_server(ServerConfig config, InternetServer server) {
	typedef struct sockaddr SocketAddress;
	typedef socklen_t SocketLength;

	// TODO: lots of continuing in this loop, needs proper error logging
	while (true) {
		SocketAddress incoming;
		SocketLength incoming_length = sizeof incoming;

		Socket connection = accept(server.socket, &incoming, &incoming_length);
		if (connection == -1) continue;

		SSL* ssl = SSL_new(server.context);
		if (ssl == NULL) continue;

		if (SSL_set_fd(ssl, connection) != 1) continue;

		SSL_set_accept_state(ssl);
		Int attempt_ssl_accept = SSL_accept(ssl);
		if (attempt_ssl_accept != 1) {
			Int ssl_error = SSL_get_error(ssl, attempt_ssl_accept);
			printf("SSL error: %d\n", ssl_error);
			shutdown(connection, 0);
		}

		Pid worker_pid;
		if (spawn_worker(ssl, config, &worker_pid) != 0) {
			SSL_shutdown(ssl);
			shutdown(connection, 0);
			continue;
		}

		close(connection);
	}
}
