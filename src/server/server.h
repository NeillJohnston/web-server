#pragma once

#include <common_types.h>
#include <netinet/ip.h>

typedef __uint16_t Port;
typedef struct sockaddr_in InternetSocketAddress;

typedef struct {
	Port port;
	Int backlog;
} ServerOptions;

typedef struct {
	FileDescriptor socket;
	InternetSocketAddress address;
} InternetServer;

static const ErrorCode ERROR_COULD_NOT_ESTABLISH = 1;
static const ErrorCode ERROR_COULD_NOT_BIND = 2;
static const ErrorCode ERROR_COULD_NOT_LISTEN = 3;
static const ErrorCode ERROR_INVALID_OPTIONS = 4;

/*
Initialize the server.
Writes details back to the provided InternetServer.
*/
ErrorCode init_server(ServerOptions options, InternetServer* server);
