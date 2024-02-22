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
bool is_variable_length_token(TokenKind kind);
std::string_view get_fixed_length_lexeme(TokenKind kind);

struct TokenHeader {
	TokenKind kind : 8 = {};
	unsigned offset : SourceOffsetBits = 0;

	bool is_variable_length() const;
};

static_assert(sizeof(TokenHeader) == 4);

struct Token {
	TokenHeader header;
	uint32_t length = 0; // will be 0 if the token kind is a fixed-length kind

	std::string_view get_lexeme(const SourceLock& source) const;
	std::string to_message_string(const SourceLock& source) const;
	std::string to_log_string(const SourceLock& source) const;
	CodeLocation locate_in(const SourceLock& source) const;
};

static_assert(sizeof(Token) == 8);

} // namespace cero
