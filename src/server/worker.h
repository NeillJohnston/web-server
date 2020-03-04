#pragma once

#include <common_types.h>

typedef FileDescriptor Socket;

/*
Forks a new worker for the specified socket.
In the parent process, writes back the child process pid to worker_pid.
In the worker process, continues to serve the request(s) over socket.
*/
ErrorCode spawn_worker(Socket socket, Pid* worker_pid);
