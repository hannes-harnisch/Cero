#pragma once

#include "cero/io/Source.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace cero {

enum class TokenKind : unsigned {
	// Variable-length tokens
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
	Ampersand,	  // &
	Pipe,		  // |
	Tilde,		  // ~
	Caret,		  // ^
	Bang,		  // !
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
	StarStar,			// **
	LeftAngleAngle,		// <<
	AmpersandAmpersand, // &&
	PipePipe,			// ||
	EqualsEquals,		// ==
	BangEquals,			// !=
	LeftAngleEquals,	// <=
	RightAngleEquals,	// >=
	PlusEquals,			// +=
	MinusEquals,		// -=
	StarEquals,			// *=
	SlashEquals,		// /=
	PercentEquals,		// %=
	AmpersandEquals,	// &=
	PipeEquals,			// |=
	TildeEquals,		// ~=

	// Three-character tokens
	Ellipsis,				  // ...
	StarStarEquals,			  // **=
	LeftAngleAngleEquals,	  // <<=
	RightAngleAngleEquals,	  // >>=
	AmpersandAmpersandEquals, // &&=
	PipePipeEquals,			  // ||=

	// Keywords
	Break,
	Catch,
	Const,
	Continue,
	Do,
	Else,
	Enum,
	For,
	If,
	In,
	Let,
	Private,
	Public,
	Return,
	Static,
	Struct,
	Switch,
	Throw,
	Try,
	Unchecked,
	Var,
	While,

	EndOfFile,
};

std::string_view token_kind_to_string(TokenKind kind);
std::string_view get_fixed_length_lexeme(TokenKind kind);

struct TokenHeader {
	TokenKind kind : 8 = {};
	SourceOffset offset : SourceOffsetBits = 0;

	CodeLocation locate_in(const SourceGuard& source) const;
};

static_assert(sizeof(TokenHeader) == 4);

} // namespace cero
