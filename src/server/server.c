#include "server.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

ErrorCode init_server(ServerConfig config, InternetServer* server) {
	typedef struct sockaddr SocketAddress;

	// Validate config
	if (config.backlog <= 0) return ERROR_INVALID_OPTIONS;

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
			ErrorCode attempt_spawn_worker = -1;
			while (attempt_spawn_worker != 0) {
				attempt_spawn_worker = spawn_worker(connection, config, &worker_pid);
			}

			// Print some information
			// TODO: have a helper process that the server can talk to, and
			// send information there instead
			if (incoming.sa_family == AF_INET) {
				Port port = ntohs(((InternetSocketAddress*) &incoming)->sin_port);
				ULong addr = ntohl(((InternetSocketAddress*) &incoming)->sin_addr.s_addr);
				printf("received connection from addr: %lx, port: %d\n", addr, port);
			}

			close(connection);
		}
	}

	// No clue how this will trigger, but including for safety
	// (I guess?)
	close(server.socket);
}
