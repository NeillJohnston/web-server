/*
Underscore: Unit Testing Framework
(Framework is a strong word, though)
*/

#pragma once

#include <stdio.h>
#include <stdbool.h>

/*
Defines how verbose the tester output should be.
Compile with -D_VERBOSE=<level> to set to a different level.

_VERBOSE=	Displays

0			Unit failures/passes (minimum)
1			Spec failures
2			Spec passes
3			Test failures
4			Test passes (maximum)
*/
#ifndef _VERBOSE
#define _VERBOSE 2
#endif

/*
Macros that define the structure of a test.

There are 4 macros that convey the test structure: UNIT, SPEC/DONE, and TEST.
There are 3 macros that run tests: ASSERT, COMPARE, and THROW.
-	ASSERT takes one argument: p.
	It fails if the predicate p tests false.
-	COMPARE takes three arguments: x, f, and y.
	It then runs f(x, y) to get a predicate result, failing if it tests false.
	Though it adds nothing, the idea is to make a completely general macro that
	reads like a custom operator, hopefully easing test case developement.
-	ERR takes one argument: r.
	The code in r is run and "expects" an error in the form of a non-zero
	return code. Does literally the same thing as ASSERT, but again, better for
	test case development.
-	OK takes on argument: r.
	Does the same thing as ERR, but expects a 0 return code.

Example usage:

UNIT(foo) {
	SPEC("must work for positive and negative bar") {
		ASSERT(foo(bar) == baz);
		ASSERT(foo(-bar) == buzz);
		DONE;
	}
	SPEC("must set one of the error flag for invalid inputs") {
		COMPARE(foo(fizz), has_some_flags_of, { .flags = ERROR1 | ERROR2; });
		DONE;
	}
}

int main() {
	TEST(foo);
}
*/

/*
Defines a unit-test function. "name" must be unique within the file.
*/
#define UNIT(name) void _unit_##name(char* _name, int* _failed_cumulative)

/*
Just an alias for main, since everything else already doesn't look like C.
*/
#define DRIVER int main()

/*
Defines a specification of the unit. "spec" should describe what functionality
the enclosed tests are meant to prove.

This macro must be followed by a block (of tests) where the last line is a
single DONE.
*/
#define SPEC(spec) { char* _spec = spec; int _failed = 0; _spec_header(_name, _spec); do

/*
Finishes a SPEC.
*/
#define DONE _spec_result(_failed); } while(0)

/*
Runs the specs within a unit. "name" should be one of the names declared in a
UNIT(name).
*/
#define TEST(name) { _unit_header(#name); int _failed = 0; _unit_##name(#name, &_failed); _unit_result(_failed); printf("\n"); }

/*
Assert that p evaluates to true.
*/
#define ASSERT(p) { bool p_ = (p); _test_assert_result(#p, !p_); if (!p_) { ++_failed; ++(*_failed_cumulative); }}

/*
Compare x and y with a binary function f that returns true if the test passes.
*/
#define COMPARE(x, f, y) { bool p = (f(x, y)); _test_compare_result(#x" "#f" "#y, !p); if (!p) { ++_failed; ++(*_failed_cumulative); }}

/*
Assert that r results in an error code (non-zero return).
*/
#define ERROR(r) { bool p = (r) == 0; _test_error_result(#r, p); if (p) { ++_failed; ++(*_failed_cumulative); }}

/*
Assert that r results in no error code (zero return).
*/
#define OKAY(r) { bool p = (r) == 0; _test_okay_result(#r, !p); if (!p) { ++_failed; ++(*_failed_cumulative); }}

#define LEAVE(msg) { ++_failed; ++(*_failed_cumulative); _leave(msg); break; }

/*
Control code macros for output and pretty printer functions.

RED, GREEN, BLUE, and WHITE are ANSI escape code sequences - as such, this test
framework only works in certain terminals. Seems to work in WSL.
*/
#define RED "\e[31m"
#define GREEN "\e[32m"
#define BLUE "\e[34m"
#define WHITE "\e[37m"

void _unit_header(char* name) {
	printf(BLUE "Unit: %s" WHITE "\n", name);
}

void _spec_header(char* name, char* spec) {
	printf(BLUE "\tSpec: \"%s %s\"" WHITE "\n", name, spec);
}

void _test_assert_result(char* test, bool failed) {
	if (_VERBOSE >= 3)
		if (failed)
			printf(RED "\t\t assert (%s): failed" WHITE "\n", test);
	if (_VERBOSE >= 4)
		if (!failed)
			printf(GREEN "\t\t assert (%s): passed" WHITE "\n", test);
}

void _test_compare_result(char* test, bool failed) {
	if (_VERBOSE >= 3)
		if (failed)
			printf(RED "\t\t compare (%s): failed" WHITE "\n", test);
	if (_VERBOSE >= 4)
		if (!failed)
			printf(GREEN "\t\t compare (%s): passed" WHITE "\n", test);
}

void _test_error_result(char* test, bool failed) {
	if (_VERBOSE >= 3)
		if (failed)
			printf(RED "\t\t expected an error (%s): failed" WHITE "\n", test);
	if (_VERBOSE >= 4)
		if (!failed)
			printf(GREEN "\t\t expected an error (%s): passed" WHITE "\n", test);
}

void _test_okay_result(char* test, bool failed) {
	if (_VERBOSE >= 3)
		if (failed)
			printf(RED "\t\t expected no error (%s): failed" WHITE "\n", test);
	if (_VERBOSE >= 4)
		if (!failed)
			printf(GREEN "\t\t expected no error (%s): passed" WHITE "\n", test);
}

void _spec_result(int failed) {
	if (_VERBOSE >= 1)
		if (failed)
			printf(RED "\tfailed %d tests" WHITE "\n", failed);
	if (_VERBOSE >= 2)
		if (!failed)
			printf(GREEN "\tpassed all tests" WHITE "\n");
}

void _unit_result(int failed) {
	if (failed)
		printf(RED "failed %d tests" WHITE "\n", failed);
	else
		printf(GREEN "passed all tests" WHITE "\n");
}

void _leave(char* message) {
	printf(RED "\t! Leaving spec: " WHITE "%s\n", message);
}