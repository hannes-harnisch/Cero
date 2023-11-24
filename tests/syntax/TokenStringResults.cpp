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

	CHECK(stream.to_string(source) == R"_____(Token stream for TokenStringForBracketsLiterals (21 tokens)
	LeftParen `(` [2:1]
	RightParen `)` [2:2]
	LeftBracket `[` [2:4]
	RightBracket `]` [2:5]
	LeftBrace `{` [2:7]
	RightBrace `}` [2:8]
	LeftAngle `<` [2:10]
	RightAngle `>` [2:11]
	Name `foo` [3:1]
	StringLiteral `"bar"` [3:5]
	CharLiteral `'baz'` [3:11]
	DecIntLiteral `123 456` [3:17]
	Semicolon `;` [3:24]
	HexIntLiteral `0x12345 456 aff` [4:1]
	Semicolon `;` [4:16]
	BinIntLiteral `0b011 01  101` [5:1]
	Semicolon `;` [5:14]
	OctIntLiteral `0o03423362 63` [6:1]
	Semicolon `;` [6:14]
	FloatLiteral `1.345634634 234623` [7:1]
	EndOfFile `` [8:1]
)_____");
}

CERO_TEST(TokenStringForOperators) {
	auto source = make_test_source(R"_____(
+ - * / % & | ~ << >>
&& || == !=
)_____");

	ExhaustiveReporter r;
	auto stream = cero::lex(source, r);

	CHECK(stream.to_string(source) == R"_____(Token stream for TokenStringForOperators (16 tokens)
	Plus `+` [2:1]
	Minus `-` [2:3]
	Star `*` [2:5]
	Slash `/` [2:7]
	Percent `%` [2:9]
	Ampersand `&` [2:11]
	Pipe `|` [2:13]
	Tilde `~` [2:15]
	LeftAngleAngle `<<` [2:17]
	RightAngle `>` [2:20]
	RightAngle `>` [2:21]
	AmpersandAmpersand `&&` [3:1]
	PipePipe `||` [3:4]
	EqualsEquals `==` [3:7]
	BangEquals `!=` [3:10]
	EndOfFile `` [4:1]
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

	CHECK(stream.to_string(source) == R"_____(Token stream for TokenStringForKeywords (19 tokens)
	Struct `struct` [2:1]
	Name `S` [2:8]
	LeftBrace `{` [3:1]
	Enum `enum` [4:5]
	Name `E` [4:10]
	LeftBrace `{` [5:5]
	RightBrace `}` [5:6]
	Name `f` [7:5]
	LeftParen `(` [7:6]
	RightParen `)` [7:7]
	ThinArrow `->` [7:9]
	Name `int32` [7:12]
	LeftBrace `{` [8:5]
	Return `return` [9:9]
	DecIntLiteral `0` [9:16]
	Semicolon `;` [9:17]
	RightBrace `}` [10:5]
	RightBrace `}` [11:1]
	EndOfFile `` [12:1]
)_____");
}
