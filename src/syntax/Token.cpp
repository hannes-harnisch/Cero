#include "Token.hpp"

#include "syntax/LexCursor.hpp"
#include "util/Fail.hpp"

namespace cero {

namespace {

	std::string_view get_token_message_format(TokenKind kind) {
		using enum TokenKind;
		switch (kind) {
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

	std::string_view to_string(TokenKind kind) {
		using enum TokenKind;
		switch (kind) {
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
			case StarStar: return "StarStar";
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
			case Extern: return "Extern";
			case For: return "For";
			case If: return "If";
			case In: return "In";
			case Let: return "Let";
			case Private: return "Private";
			case Public: return "Public";
			case Restrict: return "Restrict";
			case Return: return "Return";
			case Static: return "Static";
			case Struct: return "Struct";
			case Switch: return "Switch";
			case Throw: return "Throw";
			case Try: return "Try";
			case Use: return "Use";
			case Var: return "Var";
			case While: return "While";
			case EndOfFile: return "EndOfFile";
		}
		fail_unreachable();
	}

} // namespace

std::string_view Token::get_lexeme(const Source& source) const {
	return source.get_text().substr(offset, length);
}

std::string Token::to_message_string(const Source& source) const {
	auto format = get_token_message_format(kind);
	auto lexeme = get_lexeme(source);
	return std::vformat(format, std::make_format_args(lexeme));
}

std::string Token::to_log_string(const Source& source) const {
	auto token_kind = to_string(kind);
	auto lexeme = get_lexeme(source);
	auto location = locate_in(source);
	return std::format("{} `{}` [{}]", token_kind, lexeme, location.to_string());
}

SourceLocation Token::locate_in(const Source& source) const {
	return source.locate(offset);
}

} // namespace cero
