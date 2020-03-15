#include "../worker.c"

#include <underscore.h>
#include <unistd.h>

// Lots of stubs

ErrorCode make_http_response_string(HttpResponse response, BoundedString* response_string) {
	return 0;
}

ErrorCode read_streamed_string(FileDescriptor stream, StreamedString* string) {
	return 0;
}

ErrorCode bounded_from_streamed_string(StreamedString* streamed, BoundedString* string) {
	return 0;
}

HttpResponse respond(HttpRequest request, ServerConfig config) {
	HttpResponse response = {
		.status_code = 200
	};
	return response;
}

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