#include "syntax/LexExhaustive.hpp"
#include "util/Test.hpp"

TEST(TokenString_BracketsLiterals)
{
	auto src	= make_test_source(R"_____(
() [] {} <>
foo "bar" 'baz' 123 456
0x12345 456 aff
0b011 01  101
0o03423362 63
1.345634634 234623
)_____");
	auto stream = lex_exhaustive(src);
	auto string = stream.to_string(src);
	CHECK(string == R"_____(NewLine `` [Test_TokenString_BracketsLiterals:1:1]
LeftParen `(` [Test_TokenString_BracketsLiterals:2:1]
RightParen `)` [Test_TokenString_BracketsLiterals:2:2]
LeftBracket `[` [Test_TokenString_BracketsLiterals:2:4]
RightBracket `]` [Test_TokenString_BracketsLiterals:2:5]
LeftBrace `{` [Test_TokenString_BracketsLiterals:2:7]
RightBrace `}` [Test_TokenString_BracketsLiterals:2:8]
LeftAngle `<` [Test_TokenString_BracketsLiterals:2:10]
RightAngle `>` [Test_TokenString_BracketsLiterals:2:11]
NewLine `` [Test_TokenString_BracketsLiterals:2:12]
Name `foo` [Test_TokenString_BracketsLiterals:3:1]
StringLiteral `"bar"` [Test_TokenString_BracketsLiterals:3:5]
CharLiteral `'baz'` [Test_TokenString_BracketsLiterals:3:11]
DecIntLiteral `123 456` [Test_TokenString_BracketsLiterals:3:17]
NewLine `` [Test_TokenString_BracketsLiterals:3:24]
HexIntLiteral `0x12345 456 aff` [Test_TokenString_BracketsLiterals:4:1]
NewLine `` [Test_TokenString_BracketsLiterals:4:16]
BinIntLiteral `0b011 01  101` [Test_TokenString_BracketsLiterals:5:1]
NewLine `` [Test_TokenString_BracketsLiterals:5:14]
OctIntLiteral `0o03423362 63` [Test_TokenString_BracketsLiterals:6:1]
NewLine `` [Test_TokenString_BracketsLiterals:6:14]
FloatLiteral `1.345634634 234623` [Test_TokenString_BracketsLiterals:7:1]
NewLine `` [Test_TokenString_BracketsLiterals:7:19]
EndOfFile `` [Test_TokenString_BracketsLiterals:8:1]
)_____");
}

TEST(TokenString_Operators)
{
	auto src	= make_test_source(R"_____(
+ - * / % & | ~ << >>
&& || == !=
)_____");
	auto stream = lex_exhaustive(src);
	auto string = stream.to_string(src);
	CHECK(string == R"_____(NewLine `` [Test_TokenString_Operators:1:1]
Plus `+` [Test_TokenString_Operators:2:1]
Minus `-` [Test_TokenString_Operators:2:3]
Star `*` [Test_TokenString_Operators:2:5]
Slash `/` [Test_TokenString_Operators:2:7]
Percent `%` [Test_TokenString_Operators:2:9]
Ampersand `&` [Test_TokenString_Operators:2:11]
Pipe `|` [Test_TokenString_Operators:2:13]
Tilde `~` [Test_TokenString_Operators:2:15]
LeftAngleAngle `<<` [Test_TokenString_Operators:2:17]
RightAngleAngle `>>` [Test_TokenString_Operators:2:20]
NewLine `` [Test_TokenString_Operators:2:22]
DoubleAmpersand `&&` [Test_TokenString_Operators:3:1]
PipePipe `||` [Test_TokenString_Operators:3:4]
EqualEqual `==` [Test_TokenString_Operators:3:7]
BangEqual `!=` [Test_TokenString_Operators:3:10]
NewLine `` [Test_TokenString_Operators:3:12]
EndOfFile `` [Test_TokenString_Operators:4:1]
)_____");
}

TEST(TokenString_Keywords)
{
	auto src	= make_test_source(R"_____(
struct S
{
	enum E
	{}

	f() -> int32
	{
		return 0
	}
}
)_____");
	auto stream = lex_exhaustive(src);
	auto tokens = stream.get_tokens();
	auto string = stream.to_string(src);
	CHECK(string == R"_____(NewLine `` [Test_TokenString_Keywords:1:1]
Struct `struct` [Test_TokenString_Keywords:2:1]
Name `S` [Test_TokenString_Keywords:2:8]
NewLine `` [Test_TokenString_Keywords:2:9]
LeftBrace `{` [Test_TokenString_Keywords:3:1]
NewLine `` [Test_TokenString_Keywords:3:2]
Enum `enum` [Test_TokenString_Keywords:4:5]
Name `E` [Test_TokenString_Keywords:4:10]
NewLine `` [Test_TokenString_Keywords:4:11]
LeftBrace `{` [Test_TokenString_Keywords:5:5]
RightBrace `}` [Test_TokenString_Keywords:5:6]
NewLine `` [Test_TokenString_Keywords:5:7]
NewLine `` [Test_TokenString_Keywords:6:1]
Name `f` [Test_TokenString_Keywords:7:5]
LeftParen `(` [Test_TokenString_Keywords:7:6]
RightParen `)` [Test_TokenString_Keywords:7:7]
ThinArrow `->` [Test_TokenString_Keywords:7:9]
Name `int32` [Test_TokenString_Keywords:7:12]
NewLine `` [Test_TokenString_Keywords:7:17]
LeftBrace `{` [Test_TokenString_Keywords:8:5]
NewLine `` [Test_TokenString_Keywords:8:6]
Return `return` [Test_TokenString_Keywords:9:9]
DecIntLiteral `0` [Test_TokenString_Keywords:9:16]
NewLine `` [Test_TokenString_Keywords:9:17]
RightBrace `}` [Test_TokenString_Keywords:10:5]
NewLine `` [Test_TokenString_Keywords:10:6]
RightBrace `}` [Test_TokenString_Keywords:11:1]
NewLine `` [Test_TokenString_Keywords:11:2]
EndOfFile `` [Test_TokenString_Keywords:12:1]
)_____");
}
