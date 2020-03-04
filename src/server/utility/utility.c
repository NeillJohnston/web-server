#include "utility.h"

#include <common_types.h>

Int mod(Int a, Int m) {
	a = a % m;

	if (a >= 0) return a;
	else return a + m;
}