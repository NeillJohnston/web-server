#include "worker.h"
#include "http/http.h"
#include "string/streamed_string.h"
#include "string/bounded_string.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
Perform the task of receiving requests and sending responses.
*/
static Void be_worker(Socket socket) {
	_exit(0);
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