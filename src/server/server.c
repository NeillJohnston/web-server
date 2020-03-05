#include "server.h"
#include "worker.h"

#include <sys/socket.h>
#include <unistd.h>

ErrorCode init_server(ServerOptions options, InternetServer* server) {
	typedef struct sockaddr SocketAddress;

	// Validate options
	if (options.backlog <= 0) return ERROR_INVALID_OPTIONS;

	server->address = (InternetSocketAddress) {
		.sin_family = AF_INET,
		.sin_port = htons(options.port),
		.sin_addr.s_addr = htonl(INADDR_ANY)
	};
	server->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server->socket == -1) return ERROR_COULD_NOT_ESTABLISH;

	ErrorCode attempt_bind = bind(server->socket, (SocketAddress*) &server->address, sizeof server->address);
	if (attempt_bind != 0) return ERROR_COULD_NOT_BIND;

	ErrorCode attempt_listen = listen(server->socket, options.backlog);
	if (attempt_listen != 0) return ERROR_COULD_NOT_LISTEN;

	// TODO: start accept loop
	return 0;
}