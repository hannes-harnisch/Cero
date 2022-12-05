#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"
#include "util/TestSource.hpp"

#include <cero/syntax/Lexer.hpp>

TokenStream lex_exhaustive(const Source& source)
{
	ExhaustiveReporter reporter;
	return lex(source, reporter);
}

bool all_tokens_match(const TokenStream& token_stream, std::initializer_list<Token> kinds)
{
	auto tokens = token_stream.get_tokens();
	CHECK(tokens.size() == kinds.size());

	for (size_t i = 0; i != tokens.size(); ++i)
		if (tokens[i].kind != kinds.begin()[i])
			return false;

	return true;
}

using enum Token;

TEST(EmptySource)
{
	TestSource src("");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {EndOfFile}));
}

TEST(IntegerLiterals)
{
	TestSource src(R"_____(
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
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine,	   DecIntLiteral, NewLine,		 DecIntLiteral, NewLine,
									DecIntLiteral, NewLine,		  DecIntLiteral, NewLine,		HexIntLiteral,
									NewLine,	   HexIntLiteral, NewLine,		 HexIntLiteral, Name,
									NewLine,	   BinIntLiteral, NewLine,		 BinIntLiteral, NewLine,
									OctIntLiteral, NewLine,		  OctIntLiteral, NewLine,		EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "0");
	CHECK(tokens.at(3).get_lexeme(src) == "123");
	CHECK(tokens.at(5).get_lexeme(src) == "123 456");
	CHECK(tokens.at(7).get_lexeme(src) == "1234 5678");
	CHECK(tokens.at(9).get_lexeme(src) == "0x123 456 eaeAEB234 32 B");
	CHECK(tokens.at(11).get_lexeme(src) == "0x AB3235");
	CHECK(tokens.at(13).get_lexeme(src) == "0x AB3235");
	CHECK(tokens.at(14).get_lexeme(src) == "i");
	CHECK(tokens.at(16).get_lexeme(src) == "0b010110111");
	CHECK(tokens.at(18).get_lexeme(src) == "0b 0110 11101 110");
	CHECK(tokens.at(20).get_lexeme(src) == "0o1125417245");
	CHECK(tokens.at(22).get_lexeme(src) == "0o 124 22115 2736");
}

TEST(FloatLiterals)
{
	TestSource src(R"_____(
1.0
1.
.4
.045
100 000.000 231
123 .456 7
234 5 . 23 948
1..z
1.0.a
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, FloatLiteral, NewLine, FloatLiteral, NewLine, FloatLiteral, NewLine, FloatLiteral,
									NewLine, FloatLiteral, NewLine, FloatLiteral, NewLine, FloatLiteral, NewLine, FloatLiteral,
									Dot,	 Name,		   NewLine, FloatLiteral, Dot,	   Name,		 NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "1.0");
	CHECK(tokens.at(3).get_lexeme(src) == "1.");
	CHECK(tokens.at(5).get_lexeme(src) == ".4");
	CHECK(tokens.at(7).get_lexeme(src) == ".045");
	CHECK(tokens.at(9).get_lexeme(src) == "100 000.000 231");
	CHECK(tokens.at(11).get_lexeme(src) == "123 .456 7");
	CHECK(tokens.at(13).get_lexeme(src) == "234 5 . 23 948");
	CHECK(tokens.at(15).get_lexeme(src) == "1.");
	CHECK(tokens.at(16).get_lexeme(src) == ".");
	CHECK(tokens.at(17).get_lexeme(src) == "z");
	CHECK(tokens.at(19).get_lexeme(src) == "1.0");
	CHECK(tokens.at(20).get_lexeme(src) == ".");
	CHECK(tokens.at(21).get_lexeme(src) == "a");
}

TEST(StringLiteralsWithEscapes)
{
	TestSource src(R"_____(
"123\""
"\""
""
"\\"
"\a"
"\np"
"\"\\a\a"
"\"\\\"\\\\a\\a\""
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral, NewLine,
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

TEST(LineComments)
{
	TestSource src(R"_____(
//
// 
// abc
// //
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, LineComment, NewLine, LineComment, NewLine, LineComment, NewLine, LineComment,
									NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "//");
	CHECK(tokens.at(3).get_lexeme(src) == "// ");
	CHECK(tokens.at(5).get_lexeme(src) == "// abc");
	CHECK(tokens.at(7).get_lexeme(src) == "// //");
}

TEST(BlockComments)
{
	TestSource src(R"_____(
/**/
/* abc
*/
/*


*/
/*/**/*/
/*a/*b*/c*/
/*/*/**/*/*/
/***/
/* **** */
/*/ */
/*// */
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment,
									NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment,
									NewLine, BlockComment, NewLine, BlockComment, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "/**/");
	CHECK(tokens.at(3).get_lexeme(src) == "/* abc\n*/");
	CHECK(tokens.at(5).get_lexeme(src) == "/*\n\n\n*/");
	CHECK(tokens.at(7).get_lexeme(src) == "/*/**/*/");
	CHECK(tokens.at(9).get_lexeme(src) == "/*a/*b*/c*/");
	CHECK(tokens.at(11).get_lexeme(src) == "/*/*/**/*/*/");
	CHECK(tokens.at(13).get_lexeme(src) == "/***/");
	CHECK(tokens.at(15).get_lexeme(src) == "/* **** */");
	CHECK(tokens.at(17).get_lexeme(src) == "/*/ */");
	CHECK(tokens.at(19).get_lexeme(src) == "/*// */");
}

TEST(DotDot)
{
	TestSource src(R"_____(
..
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, Dot, Dot, NewLine, EndOfFile}));
}

TEST(BracketCaret)
{
	TestSource src(R"_____(
[^
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, LeftBracket, Caret, NewLine, EndOfFile}));
}

TEST(UnicodeNames)
{
	TestSource src(R"_____(
𖭽()
{}
)_____");
	auto	   tokens = lex_exhaustive(src);
	CHECK(all_tokens_match(tokens, {NewLine, Name, LeftParen, RightParen, NewLine, LeftBrace, RightBrace, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(src) == "𖭽");
}
