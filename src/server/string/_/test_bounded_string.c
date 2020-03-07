#include "../bounded_string.c"
// MARK Not a big fan of needing to include two C files, makes this somewhat
// of an integration test
#include "../streamed_string.c"

#include <fcntl.h>
#include <string.h>
#include <underscore.h>

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
		BoundedString lines = {
			.data = "first\nsecond\nthird",
			.length = 18
		};
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 5);
		ASSERT(strncmp(line.data, "first", 5) == 0);
		ASSERT(lines.length == 12);
		ASSERT(strncmp(lines.data, "second\nthird", 12) == 0);

		BoundedString lines_crlf = {
			.data = "first\r\nsecond\r\nthird",
			.length = 20
		};
		BoundedString line_crlf = pop_line_inplace(&lines_crlf);

		ASSERT(line_crlf.length == 5);
		ASSERT(strncmp(line_crlf.data, "first", 5) == 0);
		ASSERT(lines_crlf.length == 13);
		ASSERT(strncmp(lines_crlf.data, "second\r\nthird", 13) == 0);

		DONE;
	}
	SPEC("pops the last line") {
		BoundedString lines = {
			.data = "single line",
			.length = 11
		};
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 11);
		ASSERT(strncmp(line.data, "single line", 11) == 0);
		ASSERT(lines.length == 0);

		DONE;
	}
	SPEC("pops 0-length lines ending in LF or CRLF") {
		BoundedString lines = {
			.data = "\n\ncontent",
			.length = 9
		};
		BoundedString line = pop_line_inplace(&lines);

		ASSERT(line.length == 0);
		ASSERT(lines.length == 8);
		ASSERT(strncmp(lines.data, "\ncontent", 8) == 0);

		BoundedString lines_crlf = {
			.data = "\r\n\r\ncontent",
			.length = 11
		};
		BoundedString line_crlf = pop_line_inplace(&lines_crlf);

		ASSERT(line_crlf.length == 0);
		ASSERT(lines_crlf.length == 9);
		ASSERT(strncmp(lines_crlf.data, "\r\ncontent", 9) == 0);

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
		BoundedString lines = {
			.data = "first\nsecond\nthird",
			.length = 18
		};
		BoundedString line1 = pop_line_inplace(&lines);
		BoundedString line2 = pop_line_inplace(&lines);
		BoundedString line3 = pop_line_inplace(&lines);

		ASSERT(line1.length == 5);
		ASSERT(strncmp(line1.data, "first", 5) == 0);
		ASSERT(line2.length == 6);
		ASSERT(strncmp(line2.data, "second", 6) == 0);
		ASSERT(line3.length == 5);
		ASSERT(strncmp(line3.data, "third", 5) == 0);
		ASSERT(lines.length == 0);

		DONE;
	}
}

UNIT(pop_token_inplace) {
	SPEC("pops tokens with arbitrary whitespace") {
		BoundedString tokens = {
			.data = "first \t\r \nsecond third",
			.length = 22
		};
		BoundedString token = pop_token_inplace(&tokens);

		ASSERT(token.length == 5);
		ASSERT(strncmp(token.data, "first", 5) == 0);
		ASSERT(tokens.length == 12);
		ASSERT(strncmp(tokens.data, "second third", 12) == 0);

		DONE;
	}
	SPEC("pops the last token") {
		BoundedString tokens = {
			.data = "single_token",
			.length = 12
		};
		BoundedString token = pop_token_inplace(&tokens);

		ASSERT(token.length == 12);
		ASSERT(strncmp(token.data, "single_token", 12) == 0);
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
		BoundedString tokens = {
			.data = "a \r\nb\t c",
			.length = 8
		};
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
	TEST(free_bounded_string);
}