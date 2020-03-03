#include <underscore.h>
#include "utility.c"

UNIT(mod) {
	SPEC("yields correct answers for non-negative input") {
		ASSERT(mod(12, 3) == 0);
		ASSERT(mod(15, 4) == 3);
		ASSERT(mod(0, 1000000000) == 0);
		ASSERT(mod(1, 23) == mod(1+(4*23), 23));
		DONE;
	}
	SPEC("yields positive remainders for negative input") {
		ASSERT(mod(-1, 8) == 7);
		ASSERT(mod(-2, 2) == 0);
		ASSERT(mod(-100, 3) == 2);
		DONE;
	}
}

int main() {
	TEST(mod);
}