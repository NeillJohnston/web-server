#pragma once

#include "string/bounded_string.h"

#include <common_types.h>
#include <netinet/ip.h>

typedef __uint16_t Port;
typedef struct sockaddr_in InternetSocketAddress;
typedef FileDescriptor Socket;

typedef struct {
	// Necessary
	Port port;
	Int backlog;
	BoundedString root;
	// Optional
	Bool local;
	BoundedString api_prefix;
} ServerConfig;

typedef struct {
	FileDescriptor socket;
	InternetSocketAddress address;
} InternetServer;

static const ErrorCode ERROR_COULD_NOT_ESTABLISH = 1;
static const ErrorCode ERROR_COULD_NOT_BIND = 2;
static const ErrorCode ERROR_COULD_NOT_LISTEN = 3;
static const ErrorCode ERROR_INVALID_OPTIONS = 4;

static const ErrorCode ERROR_BAD_CONFIG_KEY = 1;
static const ErrorCode ERROR_BAD_CONFIG_VALUE = 2;
static const ErrorCode ERROR_MISSING_CONFIG_KEYS = 3;

/*
Turn the file at config_path into structured config data.
Writes back to config.
*/
ErrorCode parse_config(Char* config_path, ServerConfig* config);

/*
Initialize the server.
Writes details back to the provided InternetServer.
*/
ErrorCode init_server(ServerConfig config, InternetServer* server);

/*
Run the server.
Loops forever, accepting connections and spawning workers to fulfill requests.
Interesting info may be printed to the console.

TODO: have a helper process that the server can talk to, and send information
there instead
*/
Void run_server(ServerConfig config, InternetServer server);

/*
Forks a new worker for the specified socket.
In the parent process, writes back the child process pid to worker_pid.
In the worker process, continues to serve the request(s) over socket.
*/
ErrorCode spawn_worker(Socket socket, ServerConfig config, Pid* worker_pid);
