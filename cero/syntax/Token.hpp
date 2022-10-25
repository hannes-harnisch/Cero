#pragma once

#include <cstdint>
#include <string_view>

// Changing constant values and adding constants requires also changing the lookup table definition below.
enum class TokenKind : uint8_t
{
	Comment,
	Name,
	NewLine,
	Integer,
	Rational,
	Character,
	String,

	// One-character tokens
	Dot,		  // .
	Comma,		  // ,
	Colon,		  // :
	Semicolon,	  // ;
	LeftBrace,	  // {
	RightBrace,	  // }
	LeftParen,	  // (
	RightParen,	  // )
	LeftBracket,  // [
	RightBracket, // ]
	LeftAngle,	  // <
	RightAngle,	  // >
	Equal,		  // =
	Plus,		  // +
	Minus,		  // -
	Star,		  // *
	Slash,		  // /
	Percent,	  // %
	Bang,		  // !
	Ampersand,	  // &
	Pipe,		  // |
	Tilde,		  // ~
	Caret,		  // ^
	QuestionMark, // ?
	At,			  // @
	Dollar,		  // $
	Hash,		  // #
	Underscore,	  // _

	// Two-character tokens
	ThinArrow,		// ->
	ThickArrow,		// =>
	ColonColon,		// ::
	PlusPlus,		// ++
	MinusMinus,		// --
	TwoAmpersands,	// &&
	PipePipe,		// ||
	EqualEqual,		// ==
	BangEqual,		// !=
	LessEqual,		// <=
	GreaterEqual,	// >=
	StarStar,		// **
	TwoLeftAngles,	// <<
	TwoRightAngles, // >>
	PlusEqual,		// +=
	MinusEqual,		// -=
	StarEqual,		// *=
	SlashEqual,		// /=
	PercentEqual,	// %=
	AmpersandEqual, // &=
	PipeEqual,		// |=
	TildeEqual,		// ~=

	// Three-character tokens
	Spaceship,			// <=>
	Ellipsis,			// ...
	StarStarEqual,		// **=
	TwoLeftAngleEqual,	// <<=
	TwoRightAngleEqual, // >>=
	BracketedCaret,		// [^]

	// Keywords
	As,
	Async,
	Await,
	Break,
	Catch,
	Const,
	Continue,
	Do,
	Else,
	Enum,
	Extern,
	For,
	If,
	In,
	Let,
	Out,
	Override,
	Private,
	Protected,
	Public,
	Raw,
	Return,
	Sealed,
	Static,
	Struct,
	Super,
	Switch,
	Throw,
	Trait,
	Try,
	Use,
	Var,
	Virtual,
	While,
	Yield,

	EndOfFile,
};

struct Token
{
	TokenKind kind	 = {};
	uint16_t  length = 0;
	uint32_t  offset = 0;

	std::string_view to_string(std::string_view source) const
	{
		return source.substr(offset, length);
	}
};
