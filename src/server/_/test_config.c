#include "../config.c"
#include "../string/bounded_string.c"
#include "../string/streamed_string.c"

#include <string.h>
#include <underscore.h>

Bool equ(BoundedString bounded, Char* string) {
	if (bounded.length != strlen(string)) return false;
	return memcmp(bounded.data, string, bounded.length) == 0;
}

UNIT(parse_config) {
	SPEC("parses everything possible (including comments)") {
		ServerConfig config;

		OKAY(parse_config("src/server/_/maximum.cfg", &config));
		
		ASSERT(config.backlog == 10);
		ASSERT(config.port == 3000);
		COMPARE(config.root, equ, "out/public/");
		
		ASSERT(config.local == true);

		DONE;
	}
	SPEC("parses a minimal config file") {
		ServerConfig config;

		OKAY(parse_config("src/server/_/minimum.cfg", &config));

		ASSERT(config.backlog == 11);
		ASSERT(config.port == 3001);
		COMPARE(config.root, equ, "out/public/");

		DONE;
	}
	SPEC("errors for invalid configs") {
		ServerConfig config1;
		ERROR(parse_config("src/server/_/fail1.cfg", &config1));

		ServerConfig config2;
		ERROR(parse_config("src/server/_/fail2.cfg", &config2));
		
		ServerConfig config3;
		ERROR(parse_config("src/server/_/fail3.cfg", &config3));

		DONE;
	}
}

DRIVER {
	TEST(parse_config);
}