#include "util/ExhaustiveReporter.hpp"

#include <cero/syntax/Lexer.hpp>
#include <doctest/doctest.h>

TokenStream lex_exhaustive(const Source& source)
{
	ExhaustiveReporter reporter;
	return lex(source, reporter);
}

bool all_kinds_match(const TokenStream& token_stream, std::initializer_list<TokenKind> kinds)
{
	auto tokens = token_stream.get_slice();
	CHECK(tokens.size() == kinds.size());

	for (size_t i = 0; i != tokens.size(); ++i)
		if (tokens[i].kind != kinds.begin()[i])
			return false;

	return true;
}

using enum TokenKind;

TEST_CASE("EmptySource")
{
	Source src("");
	auto   tokens = lex_exhaustive(src);
	CHECK(all_kinds_match(tokens, {EndOfFile}));
}

TEST_CASE("NumberLiterals")
{
	Source src(R"_____(
0
123
123 456
1234 5678
0x123 456 eaeAEB234 32 B
0x AB3235
0x AB3235i
0b010110111
0b 0110 11101 110
0o1125417245
0o 124 22115 2736
1.0
1.
.4
.045
100 000.000 231
123 .456 7
234 5 . 23 948
)_____");
	auto   tokens = lex_exhaustive(src);
	CHECK(all_kinds_match(tokens, {NewLine,			DecIntLiteral,	 NewLine,		  DecIntLiteral,   NewLine,
								   DecIntLiteral,	NewLine,		 DecIntLiteral,	  NewLine,		   HexIntLiteral,
								   NewLine,			HexIntLiteral,	 NewLine,		  HexIntLiteral,   Name,
								   NewLine,			BinIntLiteral,	 NewLine,		  BinIntLiteral,   NewLine,
								   OctIntLiteral,	NewLine,		 OctIntLiteral,	  NewLine,		   DecFloatLiteral,
								   NewLine,			DecFloatLiteral, NewLine,		  DecFloatLiteral, NewLine,
								   DecFloatLiteral, NewLine,		 DecFloatLiteral, NewLine,		   DecFloatLiteral,
								   NewLine,			DecFloatLiteral, NewLine,		  EndOfFile}));
}

TEST_CASE("StringLiteralsWithEscapes")
{
	Source src(R"_____(
"123\""
"\""
""
"\\"
"\a"
"\np"
"\"\\a\a"
"\"\\\"\\\\a\\a\""
)_____");
	auto   tokens = lex_exhaustive(src);
	CHECK(all_kinds_match(tokens, {NewLine, StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral, NewLine,
								   StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral,
								   NewLine, StringLiteral, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "\"123\\\"\"");
	CHECK(tokens.at(3).get_lexeme(src) == "\"\\\"\"");
	CHECK(tokens.at(5).get_lexeme(src) == "\"\"");
	CHECK(tokens.at(7).get_lexeme(src) == "\"\\\\\"");
	CHECK(tokens.at(9).get_lexeme(src) == "\"\\a\"");
	CHECK(tokens.at(11).get_lexeme(src) == "\"\\np\"");
	CHECK(tokens.at(13).get_lexeme(src) == "\"\\\"\\\\a\\a\"");
	CHECK(tokens.at(15).get_lexeme(src) == "\"\\\"\\\\\\\"\\\\\\\\a\\\\a\\\"\"");
}

TEST_CASE("Comments")
{
	Source src(R"_____(
//
// 
// abc
// //
)_____");
	auto   tokens = lex_exhaustive(src);
	CHECK(all_kinds_match(tokens, {NewLine, LineComment, NewLine, LineComment, NewLine, LineComment, NewLine, LineComment,
								   NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "//");
	CHECK(tokens.at(3).get_lexeme(src) == "// ");
	CHECK(tokens.at(5).get_lexeme(src) == "// abc");
	CHECK(tokens.at(7).get_lexeme(src) == "// //");
}

TEST_CASE("DotDot")
{
	Source src(R"_____(
..
)_____");
	auto   tokens = lex_exhaustive(src);
	CHECK(all_kinds_match(tokens, {NewLine, Dot, Dot, NewLine, EndOfFile}));
}

TEST_CASE("BracketCaret")
{
	Source src(R"_____(
[^
)_____");
	auto   tokens = lex_exhaustive(src);
	CHECK(all_kinds_match(tokens, {NewLine, LeftBracket, Caret, NewLine, EndOfFile}));
}
