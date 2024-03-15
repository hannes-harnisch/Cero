#pragma once

#include "cero/io/Source.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace cero {

enum class TokenKind {
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
	Dot,	   // .
	Comma,	   // ,
	Colon,	   // :
	Semicolon, // ;
	LBrace,	   // {
	RBrace,	   // }
	LParen,	   // (
	RParen,	   // )
	LBracket,  // [
	RBracket,  // ]
	LAngle,	   // <
	RAngle,	   // >
	Eq,		   // =
	Plus,	   // +
	Minus,	   // -
	Star,	   // *
	Slash,	   // /
	Percent,   // %
	Amp,	   // &
	Pipe,	   // |
	Tilde,	   // ~
	Caret,	   // ^
	Bang,	   // !
	Quest,	   // ?
	At,		   // @
	Dollar,	   // $
	Hash,	   // #

	// Two-character tokens
	ThinArrow,	  // ->
	ThickArrow,	  // =>
	ColonColon,	  // ::
	PlusPlus,	  // ++
	MinusMinus,	  // --
	StarStar,	  // **
	LAngleLAngle, // <<
	AmpAmp,		  // &&
	PipePipe,	  // ||
	EqEq,		  // ==
	BangEq,		  // !=
	LAngleEq,	  // <=
	RAngleEq,	  // >=
	PlusEq,		  // +=
	MinusEq,	  // -=
	StarEq,		  // *=
	SlashEq,	  // /=
	PercentEq,	  // %=
	AmpEq,		  // &=
	PipeEq,		  // |=
	TildeEq,	  // ~=

	// Three-character tokens
	Ellipsis,		// ...
	StarStarEq,		// **=
	LAngleLAngleEq, // <<=
	RAngleRAngleEq, // >>=
	AmpAmpEq,		// &&=
	PipePipeEq,		// ||=

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
std::string_view get_token_message_format(TokenKind kind);
bool is_variable_length_token(TokenKind kind);

struct Token {
	TokenKind kind : 8 = {};
	SourceOffset offset : SourceOffsetBits = 0;

	CodeLocation locate_in(const SourceGuard& source) const;
};

static_assert(sizeof(Token) == 4);

} // namespace cero
