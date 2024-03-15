#include "Token.hpp"

#include "cero/syntax/SourceCursor.hpp"
#include "cero/util/Fail.hpp"

namespace cero {

std::string_view token_kind_to_string(TokenKind kind) {
	switch (kind) {
		using enum TokenKind;
		case Name:			 return "Name";
		case LineComment:	 return "LineComment";
		case BlockComment:	 return "BlockComment";
		case DecIntLiteral:	 return "DecIntLiteral";
		case HexIntLiteral:	 return "HexIntLiteral";
		case BinIntLiteral:	 return "BinIntLiteral";
		case OctIntLiteral:	 return "OctIntLiteral";
		case FloatLiteral:	 return "FloatLiteral";
		case CharLiteral:	 return "CharLiteral";
		case StringLiteral:	 return "StringLiteral";
		case Dot:			 return "Dot";
		case Comma:			 return "Comma";
		case Colon:			 return "Colon";
		case Semicolon:		 return "Semicolon";
		case LBrace:		 return "LBrace";
		case RBrace:		 return "RBrace";
		case LParen:		 return "LParen";
		case RParen:		 return "RParen";
		case LBracket:		 return "LBracket";
		case RBracket:		 return "RBracket";
		case LAngle:		 return "LAngle";
		case RAngle:		 return "RAngle";
		case Eq:			 return "Eq";
		case Plus:			 return "Plus";
		case Minus:			 return "Minus";
		case Star:			 return "Star";
		case Slash:			 return "Slash";
		case Percent:		 return "Percent";
		case Amp:			 return "Amp";
		case Pipe:			 return "Pipe";
		case Tilde:			 return "Tilde";
		case Caret:			 return "Caret";
		case Bang:			 return "Bang";
		case Quest:			 return "Quest";
		case At:			 return "At";
		case Dollar:		 return "Dollar";
		case Hash:			 return "Hash";
		case ThinArrow:		 return "ThinArrow";
		case ThickArrow:	 return "ThickArrow";
		case ColonColon:	 return "ColonColon";
		case PlusPlus:		 return "PlusPlus";
		case MinusMinus:	 return "MinusMinus";
		case StarStar:		 return "StarStar";
		case LAngleLAngle:	 return "LAngleLAngle";
		case AmpAmp:		 return "AmpAmp";
		case PipePipe:		 return "PipePipe";
		case EqEq:			 return "EqEq";
		case BangEq:		 return "BangEq";
		case LAngleEq:		 return "LAngleEq";
		case RAngleEq:		 return "RAngleEq";
		case PlusEq:		 return "PlusEq";
		case MinusEq:		 return "MinusEq";
		case StarEq:		 return "StarEq";
		case SlashEq:		 return "SlashEq";
		case PercentEq:		 return "PercentEq";
		case AmpEq:			 return "AmpEq";
		case PipeEq:		 return "PipeEq";
		case TildeEq:		 return "TildeEq";
		case Ellipsis:		 return "Ellipsis";
		case StarStarEq:	 return "StarStarEq";
		case LAngleLAngleEq: return "LAngleLAngleEq";
		case RAngleRAngleEq: return "RAngleRAngleEq";
		case AmpAmpEq:		 return "AmpAmpEq";
		case PipePipeEq:	 return "PipePipeEq";
		case Break:			 return "Break";
		case Catch:			 return "Catch";
		case Const:			 return "Const";
		case Continue:		 return "Continue";
		case Do:			 return "Do";
		case Else:			 return "Else";
		case Enum:			 return "Enum";
		case For:			 return "For";
		case If:			 return "If";
		case In:			 return "In";
		case Let:			 return "Let";
		case Private:		 return "Private";
		case Public:		 return "Public";
		case Return:		 return "Return";
		case Static:		 return "Static";
		case Struct:		 return "Struct";
		case Switch:		 return "Switch";
		case Throw:			 return "Throw";
		case Try:			 return "Try";
		case Unchecked:		 return "Unchecked";
		case Var:			 return "Var";
		case While:			 return "While";
		case EndOfFile:		 return "EndOfFile";
	}
	fail_unreachable();
}

std::string_view get_fixed_length_lexeme(TokenKind kind) {
	switch (kind) {
		using enum TokenKind;
		case Dot:			 return ".";
		case Comma:			 return ",";
		case Colon:			 return ":";
		case Semicolon:		 return ";";
		case LBrace:		 return "{";
		case RBrace:		 return "}";
		case LParen:		 return "(";
		case RParen:		 return ")";
		case LBracket:		 return "[";
		case RBracket:		 return "]";
		case LAngle:		 return "<";
		case RAngle:		 return ">";
		case Eq:			 return "=";
		case Plus:			 return "+";
		case Minus:			 return "-";
		case Star:			 return "*";
		case Slash:			 return "/";
		case Percent:		 return "%";
		case Amp:			 return "&";
		case Pipe:			 return "|";
		case Tilde:			 return "~";
		case Caret:			 return "^";
		case Bang:			 return "!";
		case Quest:			 return "?";
		case At:			 return "@";
		case Dollar:		 return "$";
		case Hash:			 return "#";
		case ThinArrow:		 return "->";
		case ThickArrow:	 return "=>";
		case ColonColon:	 return "::";
		case PlusPlus:		 return "++";
		case MinusMinus:	 return "--";
		case StarStar:		 return "**";
		case LAngleLAngle:	 return "<<";
		case AmpAmp:		 return "&&";
		case PipePipe:		 return "||";
		case EqEq:			 return "==";
		case BangEq:		 return "!=";
		case LAngleEq:		 return "<=";
		case RAngleEq:		 return ">=";
		case PlusEq:		 return "+=";
		case MinusEq:		 return "-=";
		case StarEq:		 return "*=";
		case SlashEq:		 return "/=";
		case PercentEq:		 return "%=";
		case AmpEq:			 return "&=";
		case PipeEq:		 return "|=";
		case TildeEq:		 return "~=";
		case Ellipsis:		 return "...";
		case StarStarEq:	 return "**=";
		case LAngleLAngleEq: return "<<=";
		case RAngleRAngleEq: return ">>=";
		case AmpAmpEq:		 return "&&=";
		case PipePipeEq:	 return "||=";
		case Break:			 return "break";
		case Catch:			 return "catch";
		case Const:			 return "const";
		case Continue:		 return "continue";
		case Do:			 return "do";
		case Else:			 return "else";
		case Enum:			 return "enum";
		case For:			 return "for";
		case If:			 return "if";
		case In:			 return "in";
		case Let:			 return "let";
		case Private:		 return "private";
		case Public:		 return "public";
		case Return:		 return "return";
		case Static:		 return "static";
		case Struct:		 return "struct";
		case Switch:		 return "switch";
		case Throw:			 return "throw";
		case Try:			 return "try";
		case Unchecked:		 return "unchecked";
		case Var:			 return "var";
		case While:			 return "while";
		case EndOfFile:		 return "";
		default:			 fail_unreachable();
	}
}

std::string_view get_token_message_format(TokenKind kind) {
	switch (kind) {
		using enum TokenKind;
		case Name: return "name `{}`";

		case LineComment:
		case BlockComment: return "comment";

		case DecIntLiteral:
		case HexIntLiteral:
		case BinIntLiteral:
		case OctIntLiteral: return "integer literal `{}`";

		case FloatLiteral:	return "floating-point literal `{}`";
		case CharLiteral:	return "character literal {}";
		case StringLiteral: return "string literal {}";
		case EndOfFile:		return "end of file";

		default: return "`{}`";
	}
}

bool is_variable_length_token(TokenKind kind) {
	switch (kind) {
		using enum TokenKind;
		case Name:
		case LineComment:
		case BlockComment:
		case DecIntLiteral:
		case HexIntLiteral:
		case BinIntLiteral:
		case OctIntLiteral:
		case FloatLiteral:
		case CharLiteral:
		case StringLiteral: return true;

		case Dot:
		case Comma:
		case Colon:
		case Semicolon:
		case LBrace:
		case RBrace:
		case LParen:
		case RParen:
		case LBracket:
		case RBracket:
		case LAngle:
		case RAngle:
		case Eq:
		case Plus:
		case Minus:
		case Star:
		case Slash:
		case Percent:
		case Amp:
		case Pipe:
		case Tilde:
		case Caret:
		case Bang:
		case Quest:
		case At:
		case Dollar:
		case Hash:
		case ThinArrow:
		case ThickArrow:
		case ColonColon:
		case PlusPlus:
		case MinusMinus:
		case StarStar:
		case LAngleLAngle:
		case AmpAmp:
		case PipePipe:
		case EqEq:
		case BangEq:
		case LAngleEq:
		case RAngleEq:
		case PlusEq:
		case MinusEq:
		case StarEq:
		case SlashEq:
		case PercentEq:
		case AmpEq:
		case PipeEq:
		case TildeEq:
		case Ellipsis:
		case StarStarEq:
		case LAngleLAngleEq:
		case RAngleRAngleEq:
		case AmpAmpEq:
		case PipePipeEq:
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
		case Private:
		case Public:
		case Return:
		case Static:
		case Struct:
		case Switch:
		case Throw:
		case Try:
		case Unchecked:
		case Var:
		case While:
		case EndOfFile:		 return false;
	}
	fail_unreachable();
}

CodeLocation Token::locate_in(const SourceGuard& source) const {
	return source.locate(offset);
}

} // namespace cero
