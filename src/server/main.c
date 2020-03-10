#include "server.h"

#include <common_types.h>
#include <stdio.h>

// TODO: move to separate module
Void log_error(Char* message) {
	fprintf(stdout, "\e[31mError: \e[37m%s\n", message);
}

ErrorCode main(int argc, char* argv[]) {
	ServerConfig options = {
		.port = 3000,
		.backlog = 1
	};
	InternetServer server;

	ErrorCode attempt_init_server = init_server(options, &server);
	if (attempt_init_server == 0)
		run_server(server);

	return 0;
}
