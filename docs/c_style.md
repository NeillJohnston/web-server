# C Style

## Preprocessor Directives

All `#include` directives should appear at the very top of the files, except for in header files where the first line should be the header guard. There should be two blocks of includes, ordered alphabetically: the first block contains project-specific includes, and the second block contains stdlib/other lib includes.

All other directives should appear in separate blocks right after the `#include` block.

Constants that may be changed at compile time should have both a `#define` and a `const` data type that uses them, where the `#define` id is prefixed with an underscore:

```C
#define _FOO 100
// ...
const Int FOO = _FOO;
```

This allows compile-time change in `gcc` via `-D_FOO=?`. Other constants should just use a `const` data type, to limit macro pollution and ensure the compiler can check types.

Using `#define` for other reasons (like functional macros) is strongly discouraged. In general, macro code is great for development tools (e.g. Underscore), but not for code in development.

## Types

All types should be defined as precisely as possible (because the language sure as hell isn't going to do that for you). Base integer types like `int` and `long` should not even be used, instead use types defined in `types.h` that provide exact representations, such as `__int32_t`.

All types should be given names (via `typedef`) before working with them, and names should be written in title case:

```C
typedef __int32_t Int
typedef __int64_t LongInt
```

Types should help abstraction, so defining types by their usage should make code more clear. Making multiple aliases for the same type is perfectly acceptable if it makes sense to do so.

```C
typedef __uint8_t Char
typedef __uint8_t Flags
```

Types should be visible to their relevant scope. If a type is only used within a single function in the file, then put the `typedef` in there.

Do not `typedef` pointer types, except for function pointers.

All of this might make it annoying (or worse, nightmareish) to interface with C libraries, but hopefully there will be minimal calls to C libs and mostly talking amongst my own code.

## Control-Flow

All control flow elements that take a condition (on in the case of a for-loop, a list of statements) should have a space to separate the keyword from its condition/statements - this is done to provide a clear separation between control-flow and function calls, which use the same syntax but are obviously very different.

```C
while (foo > 0) {
	// ...
}
```
```C
if (bar) {
	// ...
}
else baz();
```

And, as shown, chained elses get their own line. Dropping the brackets and newline for single-statement control-flow (like the `else` in the example) is fine - it improves readability (for me) because there's less whitespace and less tokens to have to understand.

Try to not mix these too much, however - if there's a single-statement `else if` in between an `if` and `else` that each have multiple lines, at least give the statement of the `else if` its own line.

Along the same lines, don't make a multi-statement control-flow block inside of a single-statement one.

```C
// Bad - just plain awkward to read
if (foo)
	while (bar) {
		// ...
	}
// Adding the "missing" brackets improves readability
if (foo) {
	while (bar) {
		// ...
	}
}
```

### For

Don't abuse for-loops.

```C
// Bad - too much unrelated stuff going on here
for (Int i = 0; foo > 0; ++bar)
```

```C
// Bad - count does not relate to the loop itself, and dropping the condition creates a subtle infinite loop
for (count = 0; ; ++count) {
	if (!foo()) break;
}

// Rewriting as a while loop improves clarity
{
	count = 0;
	while (true) {
		// ...
		++count;
	}
}
```

```C
// Bad - it's less typing, but just use a while (true)
for (;;)
```

In general, don't initialize a variable in the loop that is unrelated to the condition and update statements. If you want to use a for loop to ensure that an index/iterator variable doesn't pollute the outer scope, just make a new scope instead.

For loop are great for making code succinct, but they're also three statements in a single line - if these statements do not all relate in some way, the line becomes difficult to read.

## Variables

### Names

Keep them clear, duh.

Very short names (1 or so letters) can be used in three scenarios:

1. Canonical programming uses - e.g. `i` and `j` for loop indices.
2. Canonical mathematical uses - e.g. `x` and `y` for coordinates.
3. Documented uses - if a comment above a function explains exactly what's going on using short names and the function simply implements it in the clearest way possible, then the short names are best:

```C
/*
Euclidean algorithm for greatest common factor:
Given two integers a and b, where b < a, we can recursively break down a and b into (a = b, b = a % b) until b is 0, in which case the answer is stored in a.
*/
Int greatest_common_factor(Int a, Int b) {
	if (b == 0) return a;
	
	return greatest_common_factor(b, a % b);
}
```

### Globals

In general, no.

If a global variable _must_ be used, then it should encapsulate the program state in some way - so make a state `struct` that holds everything needed. This way the namespace gains only one item, and autocomplete will know how to handle it. This also makes it easier to capture the program state, if desired.

## Functions

Each function must be unit-tested. This means that program behavior should be split into functions that can _at least_ be unit-tested, and perhaps more splits are required - but never less.

If a function might error for any reason, it should return an error code (typically of type `ErrorCode`). Other return values should be write-back arguments at the end of the argument list. This prevents contrived special values representing errors from being made, and ensures that data written back is valid:

```C
// Instantiates a Baz with bar1 and bar2.
// Returns an error code if necessary, and writes back to baz
ErrorCode foo(Bar bar1, Bar bar2, Baz* baz) {
	// ...
}
```

Don't ignore error codes, obviously - handle them appropriately and uniquely. Uptime is important.

### Arguments

As stated before, write-back arguments of a function should be the last arguments.

Functions that are used to `free` the dynamically-allocated members of a struct should not take a pointer to the struct. This makes it clear that the argument itself will not simply be freed.

```C
// Bad - makes it look like all this function will do is call free(foo)
Void free_foo(Foo* foo);
// Taking a copy of the struct makes it clear that something more must be going on
Void free_foo(Foo foo);
```