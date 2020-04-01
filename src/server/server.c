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
	if (server->context == NULL) return ERROR_COULD_NOT_SET_UP;

	Char cert_path [PATH_MAX];
	memcpy(cert_path, config.cert_path.data, config.cert_path.length);
	cert_path[config.cert_path.length] = '\0';
	if (SSL_CTX_use_certificate_file(server->context, cert_path, SSL_FILETYPE_PEM) != 0) return ERROR_COULD_NOT_SET_UP;

	return 0;
}

Void run_server(ServerConfig config, InternetServer server) {
	typedef struct sockaddr SocketAddress;
	typedef socklen_t SocketLength;

	while (true) {
		SocketAddress incoming;
		SocketLength incoming_length = sizeof incoming;
		Pid worker_pid;

		Socket connection = accept(server.socket, &incoming, &incoming_length);

		if (connection != -1) {
			SSL* ssl_connection = SSL_new(server.context);
			if (ssl_connection != NULL) {
				ErrorCode attempt_spawn_worker = -1;
				while (attempt_spawn_worker != 0) {
					attempt_spawn_worker = spawn_worker(ssl_connection, config, &worker_pid);
				}

				close(connection);
			}
		}
	}

	// No clue how this will trigger, but including for safety
	// (I guess?)
	close(server.socket);
}
