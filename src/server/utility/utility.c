#include "utility.h"

int mod(int a, int m) {
	a = a % m;

	if (a >= 0)
		return a;
	else
		return a + m;
}