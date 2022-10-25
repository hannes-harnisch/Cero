#pragma once

#include "driver/Source.hpp"
#include "util/Enum.hpp"
#include "util/Fail.hpp"

#include <cstdint>

enum class TokenKind : unsigned
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

constexpr uint8_t get_token_length(TokenKind kind)
{
	using enum TokenKind;
	switch (kind)
	{
		// Variable-length tokens
		case Name:
		case LineComment:
		case BlockComment:
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
		case Hash:
		case Underscore: return 1;

		// Two-character tokens
		case ThinArrow:
		case ThickArrow:
		case ColonColon:
		case PlusPlus:
		case MinusMinus:
		case TwoAmpersands:
		case PipePipe:
		case EqualEqual:
		case BangEqual:
		case LessEqual:
		case GreaterEqual:
		case StarStar:
		case TwoLeftAngles:
		case TwoRightAngles:
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
		case TwoLeftAngleEqual:
		case TwoRightAngleEqual:
		case BracketedCaret: return 3;

		// Keywords
		case As:
		case Async:
		case Await:
		case Break:
		case Catch:
		case Const:
		case Continue:
		case Do:
		case Else:
		case Enum:
		case Extern:
		case For:
		case If:
		case In:
		case Let:
		case Out:
		case Override:
		case Private:
		case Protected:
		case Public:
		case Raw:
		case Return:
		case Sealed:
		case Static:
		case Struct:
		case Super:
		case Switch:
		case Throw:
		case Trait:
		case Try:
		case Use:
		case Var:
		case Virtual:
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
