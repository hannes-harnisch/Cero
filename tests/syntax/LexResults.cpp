#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Lex.hpp>
#include <cero/syntax/TokenCursor.hpp>

namespace tests {

namespace {

	void check_token_kinds(const cero::TokenStream& token_stream, std::initializer_list<cero::TokenKind> kinds) {
		REQUIRE(token_stream.num_tokens() == kinds.size());

		cero::TokenCursor c(token_stream);
		for (auto kind : kinds) {
			CHECK_EQ(kind, c.next().kind);
		}
	}

	std::string_view next_lexeme(cero::TokenCursor& cursor, const cero::SourceGuard& source) {
		auto lexeme = cursor.get_lexeme(source);
		cursor.advance();
		return lexeme;
	}

} // namespace

using enum cero::TokenKind;

CERO_TEST(LexEmptySource) {
	auto source = make_test_source("");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens, {EndOfFile});
}

CERO_TEST(LexIntegerLiterals) {
	auto source = make_test_source(R"_____(
0;
123;
123 456;
1234 5678;
0x123 456 eaeAEB234 32 B;
0x AB3235;
0x AB3235i;
0x 29356237     kk;
0b010110111;
0b 0110 11101 110;
0o1125417245;
0o 124 22115 2736;
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens,
					  {DecIntLiteral, Semicolon,	 DecIntLiteral, Semicolon,	   DecIntLiteral, Semicolon,	 DecIntLiteral,
					   Semicolon,	  HexIntLiteral, Semicolon,		HexIntLiteral, Semicolon,	  HexIntLiteral, Name,
					   Semicolon,	  HexIntLiteral, Name,			Semicolon,	   BinIntLiteral, Semicolon,	 BinIntLiteral,
					   Semicolon,	  OctIntLiteral, Semicolon,		OctIntLiteral, Semicolon,	  EndOfFile});

	cero::TokenCursor c(tokens);
	CHECK_EQ(next_lexeme(c, source), "0");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "123");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "123 456");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "1234 5678");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0x123 456 eaeAEB234 32 B");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0x AB3235");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0x AB3235");
	CHECK_EQ(next_lexeme(c, source), "i");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0x 29356237");
	CHECK_EQ(next_lexeme(c, source), "kk");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0b010110111");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0b 0110 11101 110");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0o1125417245");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "0o 124 22115 2736");
	CHECK_EQ(next_lexeme(c, source), ";");
}

CERO_TEST(LexFloatLiterals) {
	auto source = make_test_source(R"_____(
1.0;
1.;
.4;
.045;
100 000.000 231;
123 .456 7;
234 5 . 23 948;
1..z;
1.0.a;
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens, {FloatLiteral, Semicolon,		DecIntLiteral, Dot,		  Semicolon,	FloatLiteral, Semicolon,
							   FloatLiteral, Semicolon,		FloatLiteral,  Semicolon, FloatLiteral, Semicolon,	  FloatLiteral,
							   Semicolon,	 DecIntLiteral, Dot,		   Dot,		  Name,			Semicolon,	  FloatLiteral,
							   Dot,			 Name,			Semicolon,	   EndOfFile});

	cero::TokenCursor c(tokens);
	CHECK_EQ(next_lexeme(c, source), "1.0");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "1");
	CHECK_EQ(next_lexeme(c, source), ".");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), ".4");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), ".045");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "100 000.000 231");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "123 .456 7");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "234 5 . 23 948");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "1");
	CHECK_EQ(next_lexeme(c, source), ".");
	CHECK_EQ(next_lexeme(c, source), ".");
	CHECK_EQ(next_lexeme(c, source), "z");
	CHECK_EQ(next_lexeme(c, source), ";");
	CHECK_EQ(next_lexeme(c, source), "1.0");
	CHECK_EQ(next_lexeme(c, source), ".");
	CHECK_EQ(next_lexeme(c, source), "a");
	CHECK_EQ(next_lexeme(c, source), ";");
}

CERO_TEST(LexStringLiteralsWithEscapes) {
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

	check_token_kinds(tokens, {StringLiteral, StringLiteral, StringLiteral, StringLiteral, StringLiteral, StringLiteral,
							   StringLiteral, StringLiteral, EndOfFile});

	cero::TokenCursor c(tokens);
	CHECK_EQ(next_lexeme(c, source), "\"123\\\"\"");
	CHECK_EQ(next_lexeme(c, source), "\"\\\"\"");
	CHECK_EQ(next_lexeme(c, source), "\"\"");
	CHECK_EQ(next_lexeme(c, source), "\"\\\\\"");
	CHECK_EQ(next_lexeme(c, source), "\"\\a\"");
	CHECK_EQ(next_lexeme(c, source), "\"\\np\"");
	CHECK_EQ(next_lexeme(c, source), "\"\\\"\\\\a\\a\"");
	CHECK_EQ(next_lexeme(c, source), "\"\\\"\\\\\\\"\\\\\\\\a\\\\a\\\"\"");
}

CERO_TEST(LexLineComments) {
	auto source = make_test_source(R"_____(
//
// 
// abc
// //
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens, {LineComment, LineComment, LineComment, LineComment, EndOfFile});

	cero::TokenCursor c(tokens);
	CHECK_EQ(next_lexeme(c, source), "//");
	CHECK_EQ(next_lexeme(c, source), "//");
	CHECK_EQ(next_lexeme(c, source), "// abc");
	CHECK_EQ(next_lexeme(c, source), "// //");
}

CERO_TEST(LexBlockComments) {
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

	check_token_kinds(tokens, {BlockComment, BlockComment, BlockComment, BlockComment, BlockComment, BlockComment, BlockComment,
							   BlockComment, BlockComment, BlockComment, EndOfFile});

	cero::TokenCursor c(tokens);
	CHECK_EQ(next_lexeme(c, source), "/**/");
	CHECK_EQ(next_lexeme(c, source), "/* abc\n*/");
	CHECK_EQ(next_lexeme(c, source), "/*\n\n\n*/");
	CHECK_EQ(next_lexeme(c, source), "/*/**/*/");
	CHECK_EQ(next_lexeme(c, source), "/*a/*b*/c*/");
	CHECK_EQ(next_lexeme(c, source), "/*/*/**/*/*/");
	CHECK_EQ(next_lexeme(c, source), "/***/");
	CHECK_EQ(next_lexeme(c, source), "/* **** */");
	CHECK_EQ(next_lexeme(c, source), "/*/ */");
	CHECK_EQ(next_lexeme(c, source), "/*// */");
}

CERO_TEST(LexBracketCaret) {
	auto source = make_test_source(R"_____(
[^
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens, {LBracket, Caret, EndOfFile});
}

CERO_TEST(LexUnicodeNames) {
	auto source = make_test_source(R"_____(
𖭽()
{}
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens, {Name, LParen, RParen, LBrace, RBrace, EndOfFile});

	cero::TokenCursor c(tokens);
	CHECK_EQ(next_lexeme(c, source), "𖭽");
}

CERO_TEST(LexOperators) {
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

	check_token_kinds(tokens, {Bang,
							   Plus,
							   Minus,
							   Star,
							   Slash,
							   Percent,
							   EqEq,
							   BangEq,
							   LAngle,
							   RAngle,
							   LAngleEq,
							   RAngleEq,
							   Dot,
							   ColonColon,
							   AmpAmp,
							   PipePipe,
							   Eq,
							   Amp,
							   Pipe,
							   Tilde,
							   LAngleLAngle,
							   RAngle,
							   RAngle,
							   EndOfFile});
}

CERO_TEST(LexDotDot) {
	auto source = make_test_source(R"_____(
..
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	check_token_kinds(tokens, {Dot, Dot, EndOfFile});
}

} // namespace tests
