#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Lex.hpp>

namespace tests {

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
	auto tokens = cero::lex(source, r);
	CHECK(!tokens.has_errors());
	auto str = tokens.to_string(source);

	CHECK_EQ(str, R"_____(Token stream for TokenStringForBracketsLiterals (21 tokens)
	LParen `(` [2:1]
	RParen `)` [2:2]
	LBracket `[` [2:4]
	RBracket `]` [2:5]
	LBrace `{` [2:7]
	RBrace `}` [2:8]
	LAngle `<` [2:10]
	RAngle `>` [2:11]
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
	auto tokens = cero::lex(source, r);
	CHECK(!tokens.has_errors());
	auto str = tokens.to_string(source);

	CHECK_EQ(str, R"_____(Token stream for TokenStringForOperators (16 tokens)
	Plus `+` [2:1]
	Minus `-` [2:3]
	Star `*` [2:5]
	Slash `/` [2:7]
	Percent `%` [2:9]
	Amp `&` [2:11]
	Pipe `|` [2:13]
	Tilde `~` [2:15]
	LAngleLAngle `<<` [2:17]
	RAngle `>` [2:20]
	RAngle `>` [2:21]
	AmpAmp `&&` [3:1]
	PipePipe `||` [3:4]
	EqEq `==` [3:7]
	BangEq `!=` [3:10]
	EndOfFile `` [4:1]
)_____");
}

CERO_TEST(TokenStringForKeywords) {
	auto source = make_test_source(R"_____(
struct S {
	enum E {
	}

	f() -> int32 {
		return 0;
	}
}
)_____");

	ExhaustiveReporter r;
	auto tokens = cero::lex(source, r);
	CHECK(!tokens.has_errors());
	auto str = tokens.to_string(source);

	CHECK_EQ(str, R"_____(Token stream for TokenStringForKeywords (19 tokens)
	Struct `struct` [2:1]
	Name `S` [2:8]
	LBrace `{` [2:10]
	Enum `enum` [3:5]
	Name `E` [3:10]
	LBrace `{` [3:12]
	RBrace `}` [4:5]
	Name `f` [6:5]
	LParen `(` [6:6]
	RParen `)` [6:7]
	ThinArrow `->` [6:9]
	Name `int32` [6:12]
	LBrace `{` [6:18]
	Return `return` [7:9]
	DecIntLiteral `0` [7:16]
	Semicolon `;` [7:17]
	RBrace `}` [8:5]
	RBrace `}` [9:1]
	EndOfFile `` [10:1]
)_____");
}

} // namespace tests
