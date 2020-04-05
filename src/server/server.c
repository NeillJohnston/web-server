#include "server.h"

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
	if (SSL_CTX_use_certificate_file(server->context, cert_path, SSL_FILETYPE_PEM) != 1) return ERROR_SSL_CTX;

	Char pkey_path [PATH_MAX];
	memcpy(pkey_path, config.pkey_path.data, config.pkey_path.length);
	pkey_path[config.pkey_path.length] = '\0';
	if (SSL_CTX_use_PrivateKey_file(server->context, pkey_path, SSL_FILETYPE_PEM) != 1) return ERROR_SSL_CTX;

	if (SSL_CTX_check_private_key(server->context) != 1) return ERROR_SSL_CTX;

	return 0;
}

Void run_server(ServerConfig config, InternetServer server) {
	typedef struct sockaddr SocketAddress;
	typedef socklen_t SocketLength;

	while (true) {
		SocketAddress incoming;
		SocketLength incoming_length = sizeof incoming;

		Socket connection = accept(server.socket, &incoming, &incoming_length);
		if (connection != 0) continue;

		SSL* ssl = SSL_new(server.context);
		if (ssl == NULL) continue;

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
