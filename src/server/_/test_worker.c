#include "../worker.c"

#include <underscore.h>
#include <unistd.h>

// No idea how to write tests for this one (deals with sockets)
UNIT(be_worker) {
}

// Ugly to write proper tests, especially with my limited framework
UNIT(spawn_worker) {
	SPEC("must spawn a new process") {
		ServerConfig config;
		Pid current = getpid();
		Pid worker = 0;

		OKAY(spawn_worker(-1, config, &worker));
		ASSERT(worker != current);
		ASSERT(worker != 0);

		DONE;
	}
	// TODO: Can we test further?
	// SPEC("")
}

DRIVER {
	TEST(spawn_worker);
}