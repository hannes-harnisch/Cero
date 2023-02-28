#pragma once

#include <cstdint>

namespace cero
{

enum class Token : uint32_t
{
	Name,
	LineComment,
	BlockComment,
	DecIntLiteral,
	HexIntLiteral,
	BinIntLiteral,
	OctIntLiteral,
	FloatLiteral,
	CharLiteral,
	StringLiteral,

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
	Equals,		  // =
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

	// Two-character tokens
	ThinArrow,			// ->
	ThickArrow,			// =>
	ColonColon,			// ::
	PlusPlus,			// ++
	MinusMinus,			// --
	AmpersandAmpersand, // &&
	PipePipe,			// ||
	EqualsEquals,		// ==
	BangEquals,			// !=
	LeftAngleEquals,	// <=
	RightAngleEquals,	// >=
	StarStar,			// **
	LeftAngleAngle,		// <<
	PlusEquals,			// +=
	MinusEquals,		// -=
	StarEquals,			// *=
	SlashEquals,		// /=
	PercentEquals,		// %=
	AmpersandEquals,	// &=
	PipeEquals,			// |=
	TildeEquals,		// ~=

	// Three-character tokens
	Ellipsis,			   // ...
	StarStarEquals,		   // **=
	LeftAngleAngleEquals,  // <<=
	RightAngleAngleEquals, // >>=

	// Keywords
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
	Private,
	Public,
	Restrict,
	Return,
	Static,
	Struct,
	Switch,
	Throw,
	Try,
	Use,
	Var,
	While,

	EndOfFile,
};

} // namespace cero
