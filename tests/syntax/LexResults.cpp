#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Lex.hpp>

namespace
{

bool all_tokens_match(const cero::TokenStream& token_stream, std::initializer_list<cero::Token> kinds)
{
	auto tokens = token_stream.get_tokens();
	CHECK(tokens.size() == kinds.size());

	for (size_t i = 0; i != tokens.size(); ++i)
		if (tokens[i].kind != kinds.begin()[i])
			return false;

	return true;
}

} // namespace

using enum cero::Token;

CERO_TEST(LexEmptySource)
{
	auto source = make_test_source("");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {EndOfFile}));
}

CERO_TEST(LexIntegerLiterals)
{
	auto source = make_test_source(R"_____(
0
123
123 456
1234 5678
0x123 456 eaeAEB234 32 B
0x AB3235
0x AB3235i
0x 29356237     kk
0b010110111
0b 0110 11101 110
0o1125417245
0o 124 22115 2736
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, DecIntLiteral, NewLine, DecIntLiteral, NewLine,	   DecIntLiteral,
									NewLine, DecIntLiteral, NewLine, HexIntLiteral, NewLine,	   HexIntLiteral,
									NewLine, HexIntLiteral, Name,	 NewLine,		HexIntLiteral, Name,
									NewLine, BinIntLiteral, NewLine, BinIntLiteral, NewLine,	   OctIntLiteral,
									NewLine, OctIntLiteral, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(source) == "0");
	CHECK(tokens.at(3).get_lexeme(source) == "123");
	CHECK(tokens.at(5).get_lexeme(source) == "123 456");
	CHECK(tokens.at(7).get_lexeme(source) == "1234 5678");
	CHECK(tokens.at(9).get_lexeme(source) == "0x123 456 eaeAEB234 32 B");
	CHECK(tokens.at(11).get_lexeme(source) == "0x AB3235");
	CHECK(tokens.at(13).get_lexeme(source) == "0x AB3235");
	CHECK(tokens.at(14).get_lexeme(source) == "i");
	CHECK(tokens.at(16).get_lexeme(source) == "0x 29356237     ");
	CHECK(tokens.at(17).get_lexeme(source) == "kk");
	CHECK(tokens.at(19).get_lexeme(source) == "0b010110111");
	CHECK(tokens.at(21).get_lexeme(source) == "0b 0110 11101 110");
	CHECK(tokens.at(23).get_lexeme(source) == "0o1125417245");
	CHECK(tokens.at(25).get_lexeme(source) == "0o 124 22115 2736");
}

CERO_TEST(LexFloatLiterals)
{
	auto source = make_test_source(R"_____(
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

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens,
						   {NewLine,	  FloatLiteral, NewLine,	   DecIntLiteral, Dot,		NewLine,	  FloatLiteral,
							NewLine,	  FloatLiteral, NewLine,	   FloatLiteral,  NewLine,	FloatLiteral, NewLine,
							FloatLiteral, NewLine,		DecIntLiteral, Dot,			  Dot,		Name,		  NewLine,
							FloatLiteral, Dot,			Name,		   NewLine,		  EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(source) == "1.0");
	CHECK(tokens.at(3).get_lexeme(source) == "1");
	CHECK(tokens.at(4).get_lexeme(source) == ".");
	CHECK(tokens.at(6).get_lexeme(source) == ".4");
	CHECK(tokens.at(8).get_lexeme(source) == ".045");
	CHECK(tokens.at(10).get_lexeme(source) == "100 000.000 231");
	CHECK(tokens.at(12).get_lexeme(source) == "123 .456 7");
	CHECK(tokens.at(14).get_lexeme(source) == "234 5 . 23 948");
	CHECK(tokens.at(16).get_lexeme(source) == "1");
	CHECK(tokens.at(17).get_lexeme(source) == ".");
	CHECK(tokens.at(18).get_lexeme(source) == ".");
	CHECK(tokens.at(19).get_lexeme(source) == "z");
	CHECK(tokens.at(21).get_lexeme(source) == "1.0");
	CHECK(tokens.at(22).get_lexeme(source) == ".");
	CHECK(tokens.at(23).get_lexeme(source) == "a");
}

CERO_TEST(LexStringLiteralsWithEscapes)
{
	auto source = make_test_source(R"_____(
"123\""
"\""
""
"\\"
"\a"
"\np"
"\"\\a\a"
"\"\\\"\\\\a\\a\""
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral, NewLine,
									StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral, NewLine, StringLiteral,
									NewLine, StringLiteral, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(source) == "\"123\\\"\"");
	CHECK(tokens.at(3).get_lexeme(source) == "\"\\\"\"");
	CHECK(tokens.at(5).get_lexeme(source) == "\"\"");
	CHECK(tokens.at(7).get_lexeme(source) == "\"\\\\\"");
	CHECK(tokens.at(9).get_lexeme(source) == "\"\\a\"");
	CHECK(tokens.at(11).get_lexeme(source) == "\"\\np\"");
	CHECK(tokens.at(13).get_lexeme(source) == "\"\\\"\\\\a\\a\"");
	CHECK(tokens.at(15).get_lexeme(source) == "\"\\\"\\\\\\\"\\\\\\\\a\\\\a\\\"\"");
}

CERO_TEST(LexLineComments)
{
	auto source = make_test_source(R"_____(
//
// 
// abc
// //
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, LineComment, NewLine, LineComment, NewLine, LineComment, NewLine, LineComment,
									NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(source) == "//");
	CHECK(tokens.at(3).get_lexeme(source) == "// ");
	CHECK(tokens.at(5).get_lexeme(source) == "// abc");
	CHECK(tokens.at(7).get_lexeme(source) == "// //");
}

CERO_TEST(LexBlockComments)
{
	auto source = make_test_source(R"_____(
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

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment,
									NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment, NewLine, BlockComment,
									NewLine, BlockComment, NewLine, BlockComment, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(source) == "/**/");
	CHECK(tokens.at(3).get_lexeme(source) == "/* abc\n*/");
	CHECK(tokens.at(5).get_lexeme(source) == "/*\n\n\n*/");
	CHECK(tokens.at(7).get_lexeme(source) == "/*/**/*/");
	CHECK(tokens.at(9).get_lexeme(source) == "/*a/*b*/c*/");
	CHECK(tokens.at(11).get_lexeme(source) == "/*/*/**/*/*/");
	CHECK(tokens.at(13).get_lexeme(source) == "/***/");
	CHECK(tokens.at(15).get_lexeme(source) == "/* **** */");
	CHECK(tokens.at(17).get_lexeme(source) == "/*/ */");
	CHECK(tokens.at(19).get_lexeme(source) == "/*// */");
}

CERO_TEST(LexBracketCaret)
{
	auto source = make_test_source(R"_____(
[^
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, LeftBracket, Caret, NewLine, EndOfFile}));
}

CERO_TEST(LexUnicodeNames)
{
	auto source = make_test_source(R"_____(
𖭽()
{}
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, Name, LeftParen, RightParen, NewLine, LeftBrace, RightBrace, NewLine, EndOfFile}));
	CHECK(tokens.at(1).get_lexeme(source) == "𖭽");
}

CERO_TEST(LexOperators)
{
	auto source = make_test_source(R"_____(
!
+ - * / %
== != < > <= >=
.
::
&& ||
=
& | ~ << >>
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens,
						   {NewLine,		  Bang,		  NewLine,		Plus,		Minus,		Star,		Slash,
							Percent,		  NewLine,	  EqualsEquals, BangEquals, LeftAngle,	RightAngle, LeftAngleEquals,
							RightAngleEquals, NewLine,	  Dot,			NewLine,	ColonColon, NewLine,	AmpersandAmpersand,
							PipePipe,		  NewLine,	  Equals,		NewLine,	Ampersand,	Pipe,		Tilde,
							LeftAngleAngle,	  RightAngle, RightAngle,	NewLine,	EndOfFile}));
}

CERO_TEST(LexDotDot)
{
	auto source = make_test_source(R"_____(
..
)_____");

	ExhaustiveReporter r;

	auto tokens = cero::lex(source, r);
	CHECK(all_tokens_match(tokens, {NewLine, Dot, Dot, NewLine, EndOfFile}));
}
