#pragma once

#include <cstdint>

enum class TokenKind : uint32_t
{
	Name,
	NewLine,
	LineComment,
	BlockComment,
	DecIntLiteral,
	HexIntLiteral,
	BinIntLiteral,
	OctIntLiteral,
	DecFloatLiteral,
	HexFloatLiteral,
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
	ThinArrow,		  // ->
	ThickArrow,		  // =>
	ColonColon,		  // ::
	PlusPlus,		  // ++
	MinusMinus,		  // --
	DoubleAmpersand,  // &&
	PipePipe,		  // ||
	EqualEqual,		  // ==
	BangEqual,		  // !=
	LeftAngleEqual,	  // <=
	RightAngleEqual,  // >=
	StarStar,		  // **
	DoubleLeftAngle,  // <<
	DoubleRightAngle, // >>
	PlusEqual,		  // +=
	MinusEqual,		  // -=
	StarEqual,		  // *=
	SlashEqual,		  // /=
	PercentEqual,	  // %=
	AmpersandEqual,	  // &=
	PipeEqual,		  // |=
	TildeEqual,		  // ~=

	// Three-character tokens
	Ellipsis,			   // ...
	StarStarEqual,		   // **=
	DoubleLeftAngleEqual,  // <<=
	DoubleRightAngleEqual, // >>=
	BracketedCaret,		   // [^]

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
