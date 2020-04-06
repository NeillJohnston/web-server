#include "server.h"

#include <openssl/err.h>
#include <stdio.h>

static void log_error(Char* message) {
	// fprintf(stderr, "%s\n", message);
	printf("\e[31mError: \e[39m%s\n", message);
}

ErrorCode main(Int argc, Char* argv[]) {
	InternetServer server, redirect_server;
	ServerConfig config, redirect_config;

	if (argc <= 1) {
		log_error("Needs a config file (Usage: ./server <config path>");
		return -1;
	}
	Char* config_path = argv[1];

	ErrorCode attempt_parse_config = parse_config(config_path, &config);
	if (attempt_parse_config == -1) {
		log_error("Failed at parsing config, try re-running");
		return -1;
	}
	else if (attempt_parse_config == ERROR_BAD_CONFIG_KEY) {
		log_error("Bad config key (check your config file)");
		return -1;
	}
	else if (attempt_parse_config == ERROR_BAD_CONFIG_VALUE) {
		log_error("Bad config value (check your config file)");
		return -1;

	}
	else if (attempt_parse_config == ERROR_MISSING_CONFIG_KEYS) {
		log_error("Not enough values in config (check your config file)");
		return -1;
	}
	redirect_config = config;
	redirect_config.port = 80;

	ErrorCode attempt_init_server = init_server(config, &server);
	if (attempt_init_server == ERROR_COULD_NOT_BIND) {
		log_error("Could not bind address");
		return -1;
	}
	else if (attempt_init_server == ERROR_COULD_NOT_ESTABLISH) {
		log_error("Could not establish socket");
		return -1;
	}
	else if (attempt_init_server == ERROR_COULD_NOT_LISTEN) {
		log_error("Could not listen on port");
		return -1;
	}
	else if (attempt_init_server == ERROR_INVALID_OPTIONS) {
		log_error("Bad config options (check your config file)");
		return -1;
	}
	else if (attempt_init_server == ERROR_SSL_CTX) {
		log_error("Error from SSL_CTX initialiation");
		ERR_print_errors_fp(stdout);
		return -1;
	}

	// Don't need to run the redirect server if we're in dev mode
	if (!config.dev) {
		ErrorCode attempt_init_redirect_server = init_server(redirect_config, &redirect_server);
		if (attempt_init_redirect_server == ERROR_COULD_NOT_BIND) {
			log_error("Could not bind address for redirects");
			return -1;
		}
		else if (attempt_init_redirect_server == ERROR_COULD_NOT_ESTABLISH) {
			log_error("Could not establish socket for redirects");
			return -1;
		}
		else if (attempt_init_redirect_server == ERROR_COULD_NOT_LISTEN) {
			log_error("Could not listen on port for redirects");
			return -1;
		}
		else if (attempt_init_redirect_server == ERROR_INVALID_OPTIONS) {
			log_error("Bad config options (check your config file) for redirects");
			return -1;
		}
		else if (attempt_init_redirect_server == ERROR_SSL_CTX) {
			log_error("Error from SSL_CTX initialiation for redirects");
			ERR_print_errors_fp(stdout);
			return -1;
		}
		
		Pid redirect_pid;
		if (run_redirect_server(redirect_config, redirect_server, &redirect_pid) != 0) {
			log_error("Could not run redirect server");
			return -1;
		}
	}

	if (!config.dev)
		run_server(config, server);
	else
		run_dev_server(config, server);

	return 0;
}
