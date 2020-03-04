#include "server.h"

#include <common_types.h>
#include <stdio.h>

ErrorCode main(int argc, char* argv[]) {
	run_server((ServerOptions) {});
	return 0;
}
