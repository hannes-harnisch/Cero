#include "Token.hpp"

#include "cero/syntax/SourceCursor.hpp"
#include "cero/util/Fail.hpp"

namespace cero {

std::string_view token_kind_to_string(TokenKind kind) {
	switch (kind) {
		using enum TokenKind;
		case Name: return "Name";
		case LineComment: return "LineComment";
		case BlockComment: return "BlockComment";
		case DecIntLiteral: return "DecIntLiteral";
		case HexIntLiteral: return "HexIntLiteral";
		case BinIntLiteral: return "BinIntLiteral";
		case OctIntLiteral: return "OctIntLiteral";
		case FloatLiteral: return "FloatLiteral";
		case CharLiteral: return "CharLiteral";
		case StringLiteral: return "StringLiteral";
		case Dot: return "Dot";
		case Comma: return "Comma";
		case Colon: return "Colon";
		case Semicolon: return "Semicolon";
		case LeftBrace: return "LeftBrace";
		case RightBrace: return "RightBrace";
		case LeftParen: return "LeftParen";
		case RightParen: return "RightParen";
		case LeftBracket: return "LeftBracket";
		case RightBracket: return "RightBracket";
		case LeftAngle: return "LeftAngle";
		case RightAngle: return "RightAngle";
		case Equals: return "Equals";
		case Plus: return "Plus";
		case Minus: return "Minus";
		case Star: return "Star";
		case Slash: return "Slash";
		case Percent: return "Percent";
		case Bang: return "Bang";
		case Ampersand: return "Ampersand";
		case Pipe: return "Pipe";
		case Tilde: return "Tilde";
		case Caret: return "Caret";
		case QuestionMark: return "QuestionMark";
		case At: return "At";
		case Dollar: return "Dollar";
		case Hash: return "Hash";
		case ThinArrow: return "ThinArrow";
		case ThickArrow: return "ThickArrow";
		case ColonColon: return "ColonColon";
		case PlusPlus: return "PlusPlus";
		case MinusMinus: return "MinusMinus";
		case AmpersandAmpersand: return "AmpersandAmpersand";
		case PipePipe: return "PipePipe";
		case EqualsEquals: return "EqualsEquals";
		case BangEquals: return "BangEquals";
		case LeftAngleEquals: return "LeftAngleEquals";
		case RightAngleEquals: return "RightAngleEquals";
		case LeftAngleAngle: return "LeftAngleAngle";
		case PlusEquals: return "PlusEquals";
		case MinusEquals: return "MinusEquals";
		case StarEquals: return "StarEquals";
		case SlashEquals: return "SlashEquals";
		case PercentEquals: return "PercentEquals";
		case AmpersandEquals: return "AmpersandEquals";
		case PipeEquals: return "PipeEquals";
		case TildeEquals: return "TildeEquals";
		case Ellipsis: return "Ellipsis";
		case StarStarEquals: return "StarStarEquals";
		case LeftAngleAngleEquals: return "LeftAngleAngleEquals";
		case RightAngleAngleEquals: return "RightAngleAngleEquals";
		case Break: return "Break";
		case Catch: return "Catch";
		case Const: return "Const";
		case Continue: return "Continue";
		case Do: return "Do";
		case Else: return "Else";
		case Enum: return "Enum";
		case For: return "For";
		case If: return "If";
		case In: return "In";
		case Let: return "Let";
		case Private: return "Private";
		case Public: return "Public";
		case Return: return "Return";
		case Static: return "Static";
		case Struct: return "Struct";
		case Switch: return "Switch";
		case Throw: return "Throw";
		case Try: return "Try";
		case Unchecked: return "Unchecked";
		case Var: return "Var";
		case While: return "While";
		case EndOfFile: return "EndOfFile";
	}
	fail_unreachable();
}

std::string_view get_fixed_length_lexeme(TokenKind kind) {
	switch (kind) {
		using enum TokenKind;
		case Dot: return ".";
		case Comma: return ",";
		case Colon: return ":";
		case Semicolon: return ";";
		case LeftBrace: return "{";
		case RightBrace: return "}";
		case LeftParen: return "(";
		case RightParen: return ")";
		case LeftBracket: return "[";
		case RightBracket: return "]";
		case LeftAngle: return "<";
		case RightAngle: return ">";
		case Equals: return "=";
		case Plus: return "+";
		case Minus: return "-";
		case Star: return "*";
		case Slash: return "/";
		case Percent: return "%";
		case Bang: return "!";
		case Ampersand: return "&";
		case Pipe: return "|";
		case Tilde: return "~";
		case Caret: return "^";
		case QuestionMark: return "?";
		case At: return "@";
		case Dollar: return "$";
		case Hash: return "#";
		case ThinArrow: return "->";
		case ThickArrow: return "=>";
		case ColonColon: return "::";
		case PlusPlus: return "++";
		case MinusMinus: return "--";
		case StarStar: return "**";
		case AmpersandAmpersand: return "&&";
		case PipePipe: return "||";
		case EqualsEquals: return "==";
		case BangEquals: return "!=";
		case LeftAngleEquals: return "<=";
		case RightAngleEquals: return ">=";
		case LeftAngleAngle: return "<<";
		case PlusEquals: return "+=";
		case MinusEquals: return "-=";
		case StarEquals: return "*=";
		case SlashEquals: return "/=";
		case PercentEquals: return "%=";
		case AmpersandEquals: return "&=";
		case PipeEquals: return "|=";
		case TildeEquals: return "~=";
		case Ellipsis: return "...";
		case StarStarEquals: return "**=";
		case LeftAngleAngleEquals: return "<<=";
		case RightAngleAngleEquals: return ">>=";
		case Break: return "break";
		case Catch: return "catch";
		case Const: return "const";
		case Continue: return "continue";
		case Do: return "do";
		case Else: return "else";
		case Enum: return "enum";
		case For: return "for";
		case If: return "if";
		case In: return "in";
		case Let: return "let";
		case Private: return "private";
		case Public: return "public";
		case Return: return "return";
		case Static: return "static";
		case Struct: return "struct";
		case Switch: return "switch";
		case Throw: return "throw";
		case Try: return "try";
		case Unchecked: return "unchecked";
		case Var: return "var";
		case While: return "while";
		case EndOfFile: return "";
		default: return {};
	}
}

CodeLocation TokenHeader::locate_in(const SourceGuard& source) const {
	return source.locate(offset);
}

namespace {

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
			case FloatLiteral: return "floating-point literal `{}`";
			case CharLiteral: return "character literal {}";
			case StringLiteral: return "string literal {}";
			case EndOfFile: return "end of file";
			default: return "`{}`";
		}
	}

} // namespace

} // namespace cero
