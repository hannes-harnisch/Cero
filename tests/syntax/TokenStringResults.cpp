#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Lex.hpp>

CERO_TEST(TokenStringForBracketsLiterals) {
	auto source = make_test_source(R"_____(
() [] {} <>
foo "bar" 'baz' 123 456;
0x12345 456 aff;
0b011 01  101;
0o03423362 63;
1.345634634 234623
)_____");

	ExhaustiveReporter r;
	auto stream = cero::lex(source, r);

	CHECK(stream.to_string(source) == R"_____(LeftParen `(` [TokenStringForBracketsLiterals:2:1]
RightParen `)` [TokenStringForBracketsLiterals:2:2]
LeftBracket `[` [TokenStringForBracketsLiterals:2:4]
RightBracket `]` [TokenStringForBracketsLiterals:2:5]
LeftBrace `{` [TokenStringForBracketsLiterals:2:7]
RightBrace `}` [TokenStringForBracketsLiterals:2:8]
LeftAngle `<` [TokenStringForBracketsLiterals:2:10]
RightAngle `>` [TokenStringForBracketsLiterals:2:11]
Name `foo` [TokenStringForBracketsLiterals:3:1]
StringLiteral `"bar"` [TokenStringForBracketsLiterals:3:5]
CharLiteral `'baz'` [TokenStringForBracketsLiterals:3:11]
DecIntLiteral `123 456` [TokenStringForBracketsLiterals:3:17]
Semicolon `;` [TokenStringForBracketsLiterals:3:24]
HexIntLiteral `0x12345 456 aff` [TokenStringForBracketsLiterals:4:1]
Semicolon `;` [TokenStringForBracketsLiterals:4:16]
BinIntLiteral `0b011 01  101` [TokenStringForBracketsLiterals:5:1]
Semicolon `;` [TokenStringForBracketsLiterals:5:14]
OctIntLiteral `0o03423362 63` [TokenStringForBracketsLiterals:6:1]
Semicolon `;` [TokenStringForBracketsLiterals:6:14]
FloatLiteral `1.345634634 234623` [TokenStringForBracketsLiterals:7:1]
EndOfFile `` [TokenStringForBracketsLiterals:8:1]
)_____");
}

CERO_TEST(TokenStringForOperators) {
	auto source = make_test_source(R"_____(
+ - * / % & | ~ << >>
&& || == !=
)_____");

	ExhaustiveReporter r;
	auto stream = cero::lex(source, r);

	CHECK(stream.to_string(source) == R"_____(Plus `+` [TokenStringForOperators:2:1]
Minus `-` [TokenStringForOperators:2:3]
Star `*` [TokenStringForOperators:2:5]
Slash `/` [TokenStringForOperators:2:7]
Percent `%` [TokenStringForOperators:2:9]
Ampersand `&` [TokenStringForOperators:2:11]
Pipe `|` [TokenStringForOperators:2:13]
Tilde `~` [TokenStringForOperators:2:15]
LeftAngleAngle `<<` [TokenStringForOperators:2:17]
RightAngle `>` [TokenStringForOperators:2:20]
RightAngle `>` [TokenStringForOperators:2:21]
AmpersandAmpersand `&&` [TokenStringForOperators:3:1]
PipePipe `||` [TokenStringForOperators:3:4]
EqualsEquals `==` [TokenStringForOperators:3:7]
BangEquals `!=` [TokenStringForOperators:3:10]
EndOfFile `` [TokenStringForOperators:4:1]
)_____");
}

CERO_TEST(TokenStringForKeywords) {
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

	ExhaustiveReporter r;
	auto stream = cero::lex(source, r);

	CHECK(stream.to_string(source) == R"_____(Struct `struct` [TokenStringForKeywords:2:1]
Name `S` [TokenStringForKeywords:2:8]
LeftBrace `{` [TokenStringForKeywords:3:1]
Enum `enum` [TokenStringForKeywords:4:5]
Name `E` [TokenStringForKeywords:4:10]
LeftBrace `{` [TokenStringForKeywords:5:5]
RightBrace `}` [TokenStringForKeywords:5:6]
Name `f` [TokenStringForKeywords:7:5]
LeftParen `(` [TokenStringForKeywords:7:6]
RightParen `)` [TokenStringForKeywords:7:7]
ThinArrow `->` [TokenStringForKeywords:7:9]
Name `int32` [TokenStringForKeywords:7:12]
LeftBrace `{` [TokenStringForKeywords:8:5]
Return `return` [TokenStringForKeywords:9:9]
DecIntLiteral `0` [TokenStringForKeywords:9:16]
Semicolon `;` [TokenStringForKeywords:9:17]
RightBrace `}` [TokenStringForKeywords:10:5]
RightBrace `}` [TokenStringForKeywords:11:1]
EndOfFile `` [TokenStringForKeywords:12:1]
)_____");
}
