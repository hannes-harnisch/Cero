#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <syntax/Lex.hpp>

namespace {

bool all_tokens_match(const cero::TokenStream& token_stream, std::initializer_list<cero::TokenKind> kinds) {
	auto tokens = token_stream.get_tokens();
	CHECK(tokens.size() == kinds.size());

	for (size_t i = 0; i != tokens.size(); ++i)
		if (tokens[i].kind != kinds.begin()[i])
			return false;

	return true;
}

} // namespace

using enum cero::TokenKind;

CERO_TEST(LexEmptySource) {
	auto source = make_test_source("");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	CHECK(all_tokens_match(tokens, {EndOfFile}));
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

	CHECK(all_tokens_match(tokens, {DecIntLiteral, Semicolon, DecIntLiteral, Semicolon,		DecIntLiteral, Semicolon,
									DecIntLiteral, Semicolon, HexIntLiteral, Semicolon,		HexIntLiteral, Semicolon,
									HexIntLiteral, Name,	  Semicolon,	 HexIntLiteral, Name,		   Semicolon,
									BinIntLiteral, Semicolon, BinIntLiteral, Semicolon,		OctIntLiteral, Semicolon,
									OctIntLiteral, Semicolon, EndOfFile}));

	CHECK(tokens.at(0).get_lexeme(source) == "0");
	CHECK(tokens.at(2).get_lexeme(source) == "123");
	CHECK(tokens.at(4).get_lexeme(source) == "123 456");
	CHECK(tokens.at(6).get_lexeme(source) == "1234 5678");
	CHECK(tokens.at(8).get_lexeme(source) == "0x123 456 eaeAEB234 32 B");
	CHECK(tokens.at(10).get_lexeme(source) == "0x AB3235");
	CHECK(tokens.at(12).get_lexeme(source) == "0x AB3235");
	CHECK(tokens.at(13).get_lexeme(source) == "i");
	CHECK(tokens.at(15).get_lexeme(source) == "0x 29356237");
	CHECK(tokens.at(16).get_lexeme(source) == "kk");
	CHECK(tokens.at(18).get_lexeme(source) == "0b010110111");
	CHECK(tokens.at(20).get_lexeme(source) == "0b 0110 11101 110");
	CHECK(tokens.at(22).get_lexeme(source) == "0o1125417245");
	CHECK(tokens.at(24).get_lexeme(source) == "0o 124 22115 2736");
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

	CHECK(all_tokens_match(tokens,
						   {FloatLiteral, Semicolon,	 DecIntLiteral, Dot,	   Semicolon,	 FloatLiteral, Semicolon,
							FloatLiteral, Semicolon,	 FloatLiteral,	Semicolon, FloatLiteral, Semicolon,	   FloatLiteral,
							Semicolon,	  DecIntLiteral, Dot,			Dot,	   Name,		 Semicolon,	   FloatLiteral,
							Dot,		  Name,			 Semicolon,		EndOfFile}));

	CHECK(tokens.at(0).get_lexeme(source) == "1.0");
	CHECK(tokens.at(2).get_lexeme(source) == "1");
	CHECK(tokens.at(3).get_lexeme(source) == ".");
	CHECK(tokens.at(5).get_lexeme(source) == ".4");
	CHECK(tokens.at(7).get_lexeme(source) == ".045");
	CHECK(tokens.at(9).get_lexeme(source) == "100 000.000 231");
	CHECK(tokens.at(11).get_lexeme(source) == "123 .456 7");
	CHECK(tokens.at(13).get_lexeme(source) == "234 5 . 23 948");
	CHECK(tokens.at(15).get_lexeme(source) == "1");
	CHECK(tokens.at(16).get_lexeme(source) == ".");
	CHECK(tokens.at(17).get_lexeme(source) == ".");
	CHECK(tokens.at(18).get_lexeme(source) == "z");
	CHECK(tokens.at(20).get_lexeme(source) == "1.0");
	CHECK(tokens.at(21).get_lexeme(source) == ".");
	CHECK(tokens.at(22).get_lexeme(source) == "a");
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

	CHECK(all_tokens_match(tokens, {StringLiteral, StringLiteral, StringLiteral, StringLiteral, StringLiteral, StringLiteral,
									StringLiteral, StringLiteral, EndOfFile}));

	CHECK(tokens.at(0).get_lexeme(source) == "\"123\\\"\"");
	CHECK(tokens.at(1).get_lexeme(source) == "\"\\\"\"");
	CHECK(tokens.at(2).get_lexeme(source) == "\"\"");
	CHECK(tokens.at(3).get_lexeme(source) == "\"\\\\\"");
	CHECK(tokens.at(4).get_lexeme(source) == "\"\\a\"");
	CHECK(tokens.at(5).get_lexeme(source) == "\"\\np\"");
	CHECK(tokens.at(6).get_lexeme(source) == "\"\\\"\\\\a\\a\"");
	CHECK(tokens.at(7).get_lexeme(source) == "\"\\\"\\\\\\\"\\\\\\\\a\\\\a\\\"\"");
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

	CHECK(all_tokens_match(tokens, {LineComment, LineComment, LineComment, LineComment, EndOfFile}));

	CHECK(tokens.at(0).get_lexeme(source) == "//");
	CHECK(tokens.at(1).get_lexeme(source) == "// ");
	CHECK(tokens.at(2).get_lexeme(source) == "// abc");
	CHECK(tokens.at(3).get_lexeme(source) == "// //");
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

	CHECK(all_tokens_match(tokens, {BlockComment, BlockComment, BlockComment, BlockComment, BlockComment, BlockComment,
									BlockComment, BlockComment, BlockComment, BlockComment, EndOfFile}));

	CHECK(tokens.at(0).get_lexeme(source) == "/**/");
	CHECK(tokens.at(1).get_lexeme(source) == "/* abc\n*/");
	CHECK(tokens.at(2).get_lexeme(source) == "/*\n\n\n*/");
	CHECK(tokens.at(3).get_lexeme(source) == "/*/**/*/");
	CHECK(tokens.at(4).get_lexeme(source) == "/*a/*b*/c*/");
	CHECK(tokens.at(5).get_lexeme(source) == "/*/*/**/*/*/");
	CHECK(tokens.at(6).get_lexeme(source) == "/***/");
	CHECK(tokens.at(7).get_lexeme(source) == "/* **** */");
	CHECK(tokens.at(8).get_lexeme(source) == "/*/ */");
	CHECK(tokens.at(9).get_lexeme(source) == "/*// */");
}

CERO_TEST(LexBracketCaret) {
	auto source = make_test_source(R"_____(
[^
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	CHECK(all_tokens_match(tokens, {LeftBracket, Caret, EndOfFile}));
}

CERO_TEST(LexUnicodeNames) {
	auto source = make_test_source(R"_____(
𖭽()
{}
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	CHECK(all_tokens_match(tokens, {Name, LeftParen, RightParen, LeftBrace, RightBrace, EndOfFile}));

	CHECK(tokens.at(0).get_lexeme(source) == "𖭽");
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

	CHECK(all_tokens_match(tokens, {Bang,
									Plus,
									Minus,
									Star,
									Slash,
									Percent,
									EqualsEquals,
									BangEquals,
									LeftAngle,
									RightAngle,
									LeftAngleEquals,
									RightAngleEquals,
									Dot,
									ColonColon,
									AmpersandAmpersand,
									PipePipe,
									Equals,
									Ampersand,
									Pipe,
									Tilde,
									LeftAngleAngle,
									RightAngle,
									RightAngle,
									EndOfFile}));
}

CERO_TEST(LexDotDot) {
	auto source = make_test_source(R"_____(
..
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);

	CHECK(all_tokens_match(tokens, {Dot, Dot, EndOfFile}));
}
