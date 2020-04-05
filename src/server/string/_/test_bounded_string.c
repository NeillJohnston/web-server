#include "../bounded_string.c"
// MARK: Not a big fan of needing to include two C files, makes this somewhat
// of an integration test
#include "../streamed_string.c"

#include <fcntl.h>
#include <underscore.h>

BoundedString make_bounded_string(Char* data) {
	return (BoundedString) {
		.data = data,
		.length = strlen(data)
	};
}

Bool equ(BoundedString bounded, const Char* string) {
	Size length = strlen(string);
	if (bounded.length != length) return false;
	return strncmp(bounded.data, string, length) == 0;
}

UNIT(bounded_from_streamed_string) {
	SPEC("converts small strings") {
		FileDescriptor file = open("src/server/string/_/23B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 23B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		OKAY(bounded_from_streamed_string(&string, &bounded));
		ASSERT(bounded.length == 23);
		ASSERT(bounded.data[0] == '[' && bounded.data[22] == ']');

		free_streamed_string(&string);
		DONE;
	}
	SPEC("converts large strings") {
		FileDescriptor file = open("src/server/string/_/4098B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 4098B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		OKAY(bounded_from_streamed_string(&string, &bounded));
		ASSERT(bounded.length == 4098);
		ASSERT(bounded.data[0] == '[' && bounded.data[4097] == ']');

		free_streamed_string(&string);
		DONE;
	}
	SPEC("converts 0-length strings") {
		FileDescriptor file = open("src/server/string/_/0B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 0B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		OKAY(bounded_from_streamed_string(&string, &bounded));
		ASSERT(bounded.length == 0);

		free_streamed_string(&string);
		DONE;
	}
}

UNIT(bounded_string_equ) {
	SPEC("compares typical strings properly") {
		BoundedString abc = {
			.data = "abc",
			.length = 3
		};
		BoundedString abcd = {
			.data = "abcd",
			.length = 4
		};
		BoundedString abc_ = {
			.data = "abc garbage characters",
			.length = 3
		};
		BoundedString _ = {
			.data = "",
			.length = 0
		};
		BoundedString __ = {
			.data = "garbage characters",
			.length = 0
		};

		ASSERT(!bounded_string_equ(abc, abcd));
		ASSERT(!bounded_string_equ(abcd, _));
		ASSERT(bounded_string_equ(abc, abc_));
		ASSERT(bounded_string_equ(abcd, abcd));
		ASSERT(bounded_string_equ(_, __));

		DONE;
	}
	SPEC("compares strings with (null-terminators, newlines) properly") {
		BoundedString with_null = {
			.data = "hello\0world",
			.length = 11
		};
		BoundedString with_null_ = {
			.data = "hello\0world garbage characters",
			.length = 11
		};
		BoundedString with_newlines = {
			.data = "\ngoodbye\nworld",
			.length = 14
		};
		BoundedString with_newlines_ = {
			.data = "\ngoodbye\nworld garbage characters",
			.length = 14
		};

		ASSERT(bounded_string_equ(with_null, with_null_));
		ASSERT(bounded_string_equ(with_newlines, with_newlines_));
		ASSERT(!bounded_string_equ(with_null, with_newlines));

		DONE;
	}
}

UNIT(pop_line_inplace) {
	SPEC("pops typical lines ending in LF or CRLF") {
		BoundedString lines = make_bounded_string("first\nsecond\nthird");
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 5);
		ASSERT(memcmp(line.data, "first", 5) == 0);
		ASSERT(lines.length == 12);
		ASSERT(memcmp(lines.data, "second\nthird", 12) == 0);

		BoundedString lines_crlf = make_bounded_string("first\r\nsecond\r\nthird");
		BoundedString line_crlf = pop_line_inplace(&lines_crlf);

		ASSERT(line_crlf.length == 5);
		ASSERT(memcmp(line_crlf.data, "first", 5) == 0);
		ASSERT(lines_crlf.length == 13);
		ASSERT(memcmp(lines_crlf.data, "second\r\nthird", 13) == 0);

		DONE;
	}
	SPEC("pops the last line") {
		BoundedString lines = make_bounded_string("single line");
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 11);
		ASSERT(memcmp(line.data, "single line", 11) == 0);
		ASSERT(lines.length == 0);

		DONE;
	}
	SPEC("pops 0-length lines ending in LF or CRLF") {
		BoundedString lines = make_bounded_string("\n\ncontent");
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 0);
		ASSERT(lines.length == 8);
		ASSERT(memcmp(lines.data, "\ncontent", 8) == 0);

		BoundedString lines_crlf = make_bounded_string("\r\n\r\ncontent");
		BoundedString line_crlf = pop_line_inplace(&lines_crlf);

		ASSERT(line_crlf.length == 0);
		ASSERT(lines_crlf.length == 9);
		ASSERT(memcmp(lines_crlf.data, "\r\ncontent", 9) == 0);

		DONE;
	}
	SPEC("does nothing to 0-length strings") {
		BoundedString lines = {
			.data = " garbage characters",
			.length = 0
		};
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 0);
		ASSERT(lines.length == 0);

		DONE;
	}
	SPEC("pops multiple lines in sequence") {
		BoundedString lines = make_bounded_string("first\nsecond\nthird");
		BoundedString line1 = pop_line_inplace(&lines);
		BoundedString line2 = pop_line_inplace(&lines);
		BoundedString line3 = pop_line_inplace(&lines);

		ASSERT(line1.length == 5);
		ASSERT(memcmp(line1.data, "first", 5) == 0);
		ASSERT(line2.length == 6);
		ASSERT(memcmp(line2.data, "second", 6) == 0);
		ASSERT(line3.length == 5);
		ASSERT(memcmp(line3.data, "third", 5) == 0);
		ASSERT(lines.length == 0);

		DONE;
	}
}

UNIT(pop_token_inplace) {
	SPEC("pops tokens with arbitrary whitespace") {
		BoundedString tokens = make_bounded_string("first \t\r \nsecond third");
		BoundedString token = pop_token_inplace(&tokens);

		ASSERT(token.length == 5);
		ASSERT(memcmp(token.data, "first", 5) == 0);
		ASSERT(tokens.length == 12);
		ASSERT(memcmp(tokens.data, "second third", 12) == 0);

		DONE;
	}
	SPEC("pops the last token") {
		BoundedString tokens = make_bounded_string("single_token");
		BoundedString token = pop_token_inplace(&tokens);

		ASSERT(token.length == 12);
		ASSERT(memcmp(token.data, "single_token", 12) == 0);
		ASSERT(tokens.length == 0);

		DONE;
	}
	SPEC("does nothing to 0-length strings") {
		BoundedString tokens = {
			.data = " garbage characters",
			.length = 0
		};
		BoundedString token = pop_token_inplace(&tokens);

		ASSERT(token.length == 0);
		ASSERT(tokens.length == 0);

		DONE;
	}
	SPEC("pops multiple tokens in succession") {
		BoundedString tokens = make_bounded_string("a \r\nb\t c");
		BoundedString token1 = pop_token_inplace(&tokens);
		BoundedString token2 = pop_token_inplace(&tokens);
		BoundedString token3 = pop_token_inplace(&tokens);
		
		ASSERT(token1.length == 1);
		ASSERT(token1.data[0] == 'a');
		ASSERT(token2.length == 1);
		ASSERT(token2.data[0] == 'b');
		ASSERT(token3.length == 1);
		ASSERT(token3.data[0] == 'c');
		ASSERT(tokens.length == 0);

		DONE;
	}
	SPEC("returns 0-length tokens for strings with only whitespace") {
		BoundedString tokens = make_bounded_string("    ");
		BoundedString token = pop_token_inplace(&tokens);

		ASSERT(token.length == 0);
		ASSERT(tokens.length == 0);

		DONE;
	}
}

UNIT(append_inplace) {
	SPEC("appends two strings") {
		Char buffer [] = "hello.....";
		BoundedString string = {
			.data = buffer,
			.length = 5
		};
		BoundedString suffix = make_bounded_string("world");

		append_inplace(suffix, &string);
		COMPARE(string, equ, "helloworld");

		DONE;
	}
	SPEC("works with 0-length strings") {
		Char buffer1 [] = "......";
		BoundedString zero1 = {
			.data = buffer1,
			.length = 0
		};
		BoundedString string1 = make_bounded_string("string");

		append_inplace(string1, &zero1);
		COMPARE(zero1, equ, "string");

		Char buffer2 [] = "string";
		BoundedString string2 = {
			.data = buffer2,
			.length = 6
		};
		BoundedString zero2 = make_bounded_string("");

		append_inplace(zero2, &string2);
		COMPARE(string2, equ, "string");

		DONE;
	}
}

UNIT(append_cstr_inplace) {
	SPEC("appends two strings") {
		Char buffer [] = "abc....";
		BoundedString string = {
			.data = buffer,
			.length = 3
		};
		const Char* suffix = "defg";

		append_cstr_inplace(suffix, &string);
		COMPARE(string, equ, "abcdefg");

		DONE;
	}
	SPEC("works with 0-length string") {
		Char buffer1 [] = "......";
		BoundedString zero1 = {
			.data = buffer1,
			.length = 0
		};
		const Char* string1 = "string";

		append_cstr_inplace(string1, &zero1);
		COMPARE(zero1, equ, "string");

		Char buffer2 [] = "string";
		BoundedString string2 = {
			.data = buffer2,
			.length = 6
		};
		const Char* zero2 = "";

		append_cstr_inplace(zero2, &string2);
		COMPARE(string2, equ, "string");

		DONE;
	}
}

UNIT(trim_inplace) {
	SPEC("trims whitespace") {
		BoundedString string = make_bounded_string(" \t\r\n string\t \n\r\t");

		trim_inplace(&string);
		COMPARE(string, equ, "string");

		DONE;
	}
	SPEC("trims nothing if it doesn't have to") {
		BoundedString string = make_bounded_string("string");

		trim_inplace(&string);
		COMPARE(string, equ, "string");

		DONE;
	}
	SPEC("works with 0-length strings") {
		BoundedString string = make_bounded_string("");

		trim_inplace(&string);
		COMPARE(string, equ, "");

		DONE;
	}
	SPEC("works with all-whitespace strings") {
		BoundedString string = make_bounded_string(" \r\t\n ");

		trim_inplace(&string);
		COMPARE(string, equ, "");

		DONE;
	}
}

UNIT(copy_bounded_string) {
	SPEC("produces an identical string with new memory") {
		BoundedString source = make_bounded_string("hello");
		BoundedString destination;

		if (copy_bounded_string(source, &destination) != 0) LEAVE("malloc error in copy_bounded_string");

		ASSERT(source.length == destination.length);
		ASSERT(memcmp(source.data, destination.data, source.length) == 0);
		// Will crash if destination.data was not dynamically allocated
		free(destination.data);
		
		DONE;
	}
}

UNIT(free_bounded_string) {
	SPEC("frees properly") {
		FileDescriptor file = open("src/server/string/_/4098B.txt", O_RDONLY);
		if (file == -1) LEAVE("could not open 4098B.txt");
		StreamedString string;
		if (read_streamed_string(file, &string) != 0) LEAVE("could not read streamed string");

		BoundedString bounded;
		bounded_from_streamed_string(&string, &bounded);
		free_bounded_string(bounded);
		// If there's an error the program will just crash

		free_streamed_string(&string);
		DONE;
	}
}

DRIVER {
	TEST(bounded_from_streamed_string);
	TEST(bounded_string_equ);
	TEST(pop_line_inplace);
	TEST(pop_token_inplace);
	TEST(append_inplace);
	TEST(append_cstr_inplace);
	TEST(trim_inplace);
	TEST(copy_bounded_string);
	TEST(free_bounded_string);
}

// Blank stubs
Int SSL_read(SSL* ssl, Void* buffer, Int num) {
	return 0;
}