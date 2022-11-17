#pragma once

#include "driver/Source.hpp"

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

struct Token
{
	static constexpr size_t KIND_BITS	= 8;
	static constexpr size_t LENGTH_BITS = 24;
	static constexpr size_t MAX_LENGTH	= (1 << LENGTH_BITS) - 1;

	TokenKind kind : KIND_BITS	   = {};
	uint32_t  length : LENGTH_BITS = 0;
	uint32_t  offset			   = 0;

	std::string_view get_lexeme_from(const Source& source) const;
	std::string		 describe_for_message(const Source& source) const;
	std::string		 to_string(const Source& source) const;
	SourceLocation	 locate_in(const Source& source) const;
};
