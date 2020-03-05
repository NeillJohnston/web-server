#include "../server.c"

#include <underscore.h>

UNIT(init_server) {
	SPEC("is able to initialize a server") {
		ServerOptions options = {
			.port = 3000,
			.backlog = 10
		};
		InternetServer temp;

		OKAY(init_server(options, &temp));

		DONE;
	}
	SPEC("errors for invalid options") {
		ServerOptions options = {
			.port = 3001,
			.backlog = -1
		};
		InternetServer temp;

		ERROR(init_server(options, &temp));

		DONE;
	}
	SPEC("will not bind to the same port twice") {
		ServerOptions options = {
			.port = 3002,
			.backlog = 10
		};
		InternetServer temp1;
		InternetServer temp2;

		OKAY(init_server(options, &temp1));
		ASSERT(init_server(options, &temp2) == ERROR_COULD_NOT_BIND);
		
		DONE;
	}
}

DRIVER {
	TEST(init_server);
}