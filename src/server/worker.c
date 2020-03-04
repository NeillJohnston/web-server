#include "worker.h"

#include <unistd.h>

Void be_worker(Socket socket) {
	// TODO:
	_exit(-1);
}

ErrorCode spawn_worker(Socket socket, Pid* worker_pid) {
	const Pid INVALID = -1;

	Pid forked = fork();
	
	if (forked == INVALID) return -1;
	else if (forked == 0) {
		be_worker(socket);
	}
	else {
		*worker_pid = forked;
	}

	return 0;
}