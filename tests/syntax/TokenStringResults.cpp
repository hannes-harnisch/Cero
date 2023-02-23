#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Lex.hpp>

CERO_TEST(TokenStringForBracketsLiterals)
{
	ExhaustiveReporter r;

	auto source = make_test_source(R"_____(
() [] {} <>
foo "bar" 'baz' 123 456
0x12345 456 aff
0b011 01  101
0o03423362 63
1.345634634 234623
)_____");

	auto stream = cero::lex(source, r);
	CHECK(stream.to_string(source) == R"_____(LeftParen `(` [Test_TokenStringForBracketsLiterals:2:1]
RightParen `)` [Test_TokenStringForBracketsLiterals:2:2]
LeftBracket `[` [Test_TokenStringForBracketsLiterals:2:4]
RightBracket `]` [Test_TokenStringForBracketsLiterals:2:5]
LeftBrace `{` [Test_TokenStringForBracketsLiterals:2:7]
RightBrace `}` [Test_TokenStringForBracketsLiterals:2:8]
LeftAngle `<` [Test_TokenStringForBracketsLiterals:2:10]
RightAngle `>` [Test_TokenStringForBracketsLiterals:2:11]
Name `foo` [Test_TokenStringForBracketsLiterals:3:1]
StringLiteral `"bar"` [Test_TokenStringForBracketsLiterals:3:5]
CharLiteral `'baz'` [Test_TokenStringForBracketsLiterals:3:11]
DecIntLiteral `123 456` [Test_TokenStringForBracketsLiterals:3:17]
HexIntLiteral `0x12345 456 aff` [Test_TokenStringForBracketsLiterals:4:1]
BinIntLiteral `0b011 01  101` [Test_TokenStringForBracketsLiterals:5:1]
OctIntLiteral `0o03423362 63` [Test_TokenStringForBracketsLiterals:6:1]
FloatLiteral `1.345634634 234623` [Test_TokenStringForBracketsLiterals:7:1]
EndOfFile `` [Test_TokenStringForBracketsLiterals:8:1]
)_____");
}

CERO_TEST(TokenStringForOperators)
{
	ExhaustiveReporter r;

	auto source = make_test_source(R"_____(
+ - * / % & | ~ << >>
&& || == !=
)_____");

	auto stream = cero::lex(source, r);
	CHECK(stream.to_string(source) == R"_____(Plus `+` [Test_TokenStringForOperators:2:1]
Minus `-` [Test_TokenStringForOperators:2:3]
Star `*` [Test_TokenStringForOperators:2:5]
Slash `/` [Test_TokenStringForOperators:2:7]
Percent `%` [Test_TokenStringForOperators:2:9]
Ampersand `&` [Test_TokenStringForOperators:2:11]
Pipe `|` [Test_TokenStringForOperators:2:13]
Tilde `~` [Test_TokenStringForOperators:2:15]
LeftAngleAngle `<<` [Test_TokenStringForOperators:2:17]
RightAngle `>` [Test_TokenStringForOperators:2:20]
RightAngle `>` [Test_TokenStringForOperators:2:21]
AmpersandAmpersand `&&` [Test_TokenStringForOperators:3:1]
PipePipe `||` [Test_TokenStringForOperators:3:4]
EqualsEquals `==` [Test_TokenStringForOperators:3:7]
BangEquals `!=` [Test_TokenStringForOperators:3:10]
EndOfFile `` [Test_TokenStringForOperators:4:1]
)_____");
}

CERO_TEST(TokenStringForKeywords)
{
	ExhaustiveReporter r;

	auto source = make_test_source(R"_____(
struct S
{
	enum E
	{}

	f() -> int32
	{
		return 0;
	}
}
)_____");

	auto stream = cero::lex(source, r);
	CHECK(stream.to_string(source) == R"_____(Struct `struct` [Test_TokenStringForKeywords:2:1]
Name `S` [Test_TokenStringForKeywords:2:8]
LeftBrace `{` [Test_TokenStringForKeywords:3:1]
Enum `enum` [Test_TokenStringForKeywords:4:5]
Name `E` [Test_TokenStringForKeywords:4:10]
LeftBrace `{` [Test_TokenStringForKeywords:5:5]
RightBrace `}` [Test_TokenStringForKeywords:5:6]
Name `f` [Test_TokenStringForKeywords:7:5]
LeftParen `(` [Test_TokenStringForKeywords:7:6]
RightParen `)` [Test_TokenStringForKeywords:7:7]
ThinArrow `->` [Test_TokenStringForKeywords:7:9]
Name `int32` [Test_TokenStringForKeywords:7:12]
LeftBrace `{` [Test_TokenStringForKeywords:8:5]
Return `return` [Test_TokenStringForKeywords:9:9]
DecIntLiteral `0` [Test_TokenStringForKeywords:9:16]
Semicolon `;` [Test_TokenStringForKeywords:9:17]
RightBrace `}` [Test_TokenStringForKeywords:10:5]
RightBrace `}` [Test_TokenStringForKeywords:11:1]
EndOfFile `` [Test_TokenStringForKeywords:12:1]
)_____");
}
