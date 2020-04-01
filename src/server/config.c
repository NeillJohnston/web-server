#include "server.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

typedef __uint64_t Flags;

static const Char* KEY_PORT = "port";
static const Char* KEY_ROOT = "root";
static const Char* KEY_DB_PATH = "db";
static const Char* KEY_CERT_PATH = "cert";

static const Char* KEY_BACKLOG = "back";
static const Char* KEY_LOCAL = "local";

// Flags for all of the necessary variables
static const Flags FLAG_PORT = 1<<0;
static const Flags FLAG_ROOT = 1<<1;
static const Flags FLAG_DB_PATH = 1<<2;
static const Flags FLAG_CERT_PATH = 1<<3;

static const Char* VALUE_TRUE = "true";
static const Char* VALUE_FALSE = "false";

/*
Return whether a BoundedString is equal to a C-string.
*/
static Bool bounded_equ_cstr(BoundedString bounded, const Char* cstr) {
	if (bounded.length != strlen(cstr)) return false;
	
	return memcmp(bounded.data, cstr, bounded.length) == 0;
}

/*
Unset the given flag in a mask.
*/
static Flags unset(Flags mask, Flags flag) {
	return mask & ~flag;
}

ErrorCode parse_config(Char* path, ServerConfig* config) {
	FileDescriptor config_file = open(path, O_RDONLY);
	StreamedString streamed_config_string;
	if (read_streamed_string(config_file, &streamed_config_string) != 0) return -1;
	BoundedString config_string;
	if (bounded_from_streamed_string(&streamed_config_string, &config_string) != 0) return -1;
	BoundedString mut_config_string = config_string;

	// Initialize optionals with defaults
	config->backlog = 10;
	config->local = true;

	Flags required = FLAG_PORT | FLAG_ROOT | FLAG_DB_PATH;

	while (mut_config_string.length > 0) {
		BoundedString pair = pop_line_inplace(&mut_config_string);
		BoundedString key = pop_token_inplace(&pair);
		BoundedString value = pair;

		// Skips blank lines and comments
		if (key.length == 0) continue;
		else if (key.data[0] == '#') continue;

		if (bounded_equ_cstr(key, KEY_PORT)) {
			if (sscanf(value.data, "%hu", &config->port) == 0) return ERROR_BAD_CONFIG_VALUE;
			required = unset(required, FLAG_PORT);
		}
		else if (bounded_equ_cstr(key, KEY_ROOT)) {
			if (copy_bounded_string(value, &config->root) != 0) return -1;
			required = unset(required, FLAG_ROOT);
		}
		else if (bounded_equ_cstr(key, KEY_DB_PATH)) {
			if (copy_bounded_string(value, &config->db_path) != 0) return -1;
			required = unset(required, FLAG_DB_PATH);
		}
		else if (bounded_equ_cstr(key, KEY_CERT_PATH)) {
			if (copy_bounded_string(value, &config->cert_path) != 0) return -1;
			required = unset(required, FLAG_CERT_PATH);
		}
		else if (bounded_equ_cstr(key, KEY_BACKLOG)) {
			if (sscanf(value.data, "%d", &config->backlog) == 0) return ERROR_BAD_CONFIG_VALUE;
		}
		else if (bounded_equ_cstr(key, KEY_LOCAL)) {
			if (bounded_equ_cstr(value, VALUE_TRUE)) {
				config->local = true;
			}
			else if (bounded_equ_cstr(value, VALUE_FALSE)) {
				config->local = false;
			}
			else {
				return ERROR_BAD_CONFIG_VALUE;
			}
		}
		else {
			return ERROR_BAD_CONFIG_KEY;
		}
	}

	close(config_file);
	free_streamed_string(&streamed_config_string);
	free_bounded_string(config_string);

	if (required != 0) return ERROR_MISSING_CONFIG_KEYS;
	return 0;
}