#include <common_types.h>
#include <stdio.h>

ErrorCode main(int argc, char* argv[]) {
	printf("%s\n", argv[argc-1]);
	return 0;
}
