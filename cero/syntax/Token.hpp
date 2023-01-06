#pragma once

#include <cstdint>

enum class Token : uint32_t
{
	Name,
	NewLine,
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

	// Two-character tokens
	ThinArrow,		 // ->
	ThickArrow,		 // =>
	ColonColon,		 // ::
	PlusPlus,		 // ++
	MinusMinus,		 // --
	DoubleAmpersand, // &&
	PipePipe,		 // ||
	EqualEqual,		 // ==
	BangEqual,		 // !=
	LeftAngleEqual,	 // <=
	RightAngleEqual, // >=
	StarStar,		 // **
	LeftAngleAngle,	 // <<
	RightAngleAngle, // >>
	PlusEqual,		 // +=
	MinusEqual,		 // -=
	StarEqual,		 // *=
	SlashEqual,		 // /=
	PercentEqual,	 // %=
	AmpersandEqual,	 // &=
	PipeEqual,		 // |=
	TildeEqual,		 // ~=

	// Three-character tokens
	Ellipsis,			  // ...
	StarStarEqual,		  // **=
	LeftAngleAngleEqual,  // <<=
	RightAngleAngleEqual, // >>=

	// Keywords
	Break,
	Catch,
	Const,
	Continue,
	Else,
	Enum,
	For,
	If,
	In,
	Let,
	Public,
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
