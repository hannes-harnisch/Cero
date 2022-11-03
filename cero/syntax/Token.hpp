#pragma once

#include "driver/Source.hpp"
#include "util/Enum.hpp"
#include "util/Fail.hpp"

#include <cstdint>

enum class TokenKind : uint32_t
{
	Name,
	NewLine,
	LineComment,
	BlockComment,
	DocComment,
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
	LessEqual,		  // <=
	GreaterEqual,	  // >=
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
	Spaceship,			   // <=>
	Ellipsis,			   // ...
	StarStarEqual,		   // **=
	DoubleLeftAngleEqual,  // <<=
	DoubleRightAngleEqual, // >>=
	BracketedCaret,		   // [^]

	// Keywords
	Await,
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
	Yield,

	EndOfFile,
};

constexpr uint8_t get_token_length(TokenKind kind)
{
	using enum TokenKind;
	switch (kind)
	{
		// Variable-length tokens
		case Name:
		case LineComment:
		case BlockComment:
		case DocComment:
		case DecIntLiteral:
		case HexIntLiteral:
		case BinIntLiteral:
		case OctIntLiteral:
		case DecFloatLiteral:
		case HexFloatLiteral:
		case CharLiteral:
		case StringLiteral: return 0xff;

		// Zero-character tokens
		case EndOfFile: return 0;

		// One-character tokens
		case NewLine:
		case Dot:
		case Comma:
		case Colon:
		case Semicolon:
		case LeftBrace:
		case RightBrace:
		case LeftParen:
		case RightParen:
		case LeftBracket:
		case RightBracket:
		case LeftAngle:
		case RightAngle:
		case Equal:
		case Plus:
		case Minus:
		case Star:
		case Slash:
		case Percent:
		case Bang:
		case Ampersand:
		case Pipe:
		case Tilde:
		case Caret:
		case QuestionMark:
		case At:
		case Dollar:
		case Hash: return 1;

		// Two-character tokens
		case ThinArrow:
		case ThickArrow:
		case ColonColon:
		case PlusPlus:
		case MinusMinus:
		case DoubleAmpersand:
		case PipePipe:
		case EqualEqual:
		case BangEqual:
		case LessEqual:
		case GreaterEqual:
		case StarStar:
		case DoubleLeftAngle:
		case DoubleRightAngle:
		case PlusEqual:
		case MinusEqual:
		case StarEqual:
		case SlashEqual:
		case PercentEqual:
		case AmpersandEqual:
		case PipeEqual:
		case TildeEqual: return 2;

		// Three-character tokens
		case Spaceship:
		case Ellipsis:
		case StarStarEqual:
		case DoubleLeftAngleEqual:
		case DoubleRightAngleEqual:
		case BracketedCaret: return 3;

		// Keywords
		case Await:
		case Break:
		case Catch:
		case Const:
		case Continue:
		case Do:
		case Else:
		case Enum:
		case For:
		case If:
		case In:
		case Let:
		case Public:
		case Return:
		case Static:
		case Struct:
		case Switch:
		case Throw:
		case Try:
		case Use:
		case Var:
		case While:
		case Yield: return static_cast<uint8_t>(magic_enum::enum_name(kind).length());
	}
	fail_unreachable();
}

bool is_variable_length_token(TokenKind kind);

struct Token
{
	static constexpr size_t KIND_BITS	= 8;
	static constexpr size_t LENGTH_BITS = 24;
	static constexpr size_t MAX_LENGTH	= (1 << LENGTH_BITS) - 1;

	TokenKind kind : KIND_BITS	   = {};
	uint32_t  length : LENGTH_BITS = 0;
	uint32_t  offset			   = 0;

	std::string_view get_lexeme(const Source& source) const;
	std::string		 to_string(const Source& source) const;
};
