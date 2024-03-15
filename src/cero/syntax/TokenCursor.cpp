#include "TokenCursor.hpp"

namespace cero {

namespace {

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
			case LeftBrace:
			case RightBrace:
			case LeftParen:
			case RightParen:
			case LeftBracket:
			case RightBracket:
			case LeftAngle:
			case RightAngle:
			case Equals:
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
			case ThinArrow:
			case ThickArrow:
			case ColonColon:
			case PlusPlus:
			case MinusMinus:
			case StarStar:
			case AmpersandAmpersand:
			case PipePipe:
			case EqualsEquals:
			case BangEquals:
			case LeftAngleEquals:
			case RightAngleEquals:
			case LeftAngleAngle:
			case PlusEquals:
			case MinusEquals:
			case StarEquals:
			case SlashEquals:
			case PercentEquals:
			case AmpersandEquals:
			case PipeEquals:
			case TildeEquals:
			case Ellipsis:
			case StarStarEquals:
			case LeftAngleAngleEquals:
			case RightAngleAngleEquals:
			case AmpersandAmpersandEquals:
			case PipePipeEquals:
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
			case EndOfFile: return false;
		}
		return false;
	}

} // namespace

TokenCursor::TokenCursor(const TokenStream& token_stream) :
	it_(token_stream.raw().begin()) {
}

TokenHeader TokenCursor::next() {
	const auto header = it_->header;

	if (header.kind != TokenKind::EndOfFile) {
		it_ += is_variable_length_token(header.kind) ? 2 : 1;
	}

	return header;
}

bool TokenCursor::match(TokenKind kind) {
	auto token = peek();
	if (token.kind == kind) {
		advance();
		return true;
	}
	return false;
}

std::optional<TokenHeader> TokenCursor::match_token(TokenKind kind) {
	auto token = peek();
	if (token.kind == kind) {
		advance();
		return token;
	}
	return std::nullopt;
}

std::optional<TokenHeader> TokenCursor::match_name() {
	auto token = peek();
	if (token.kind == TokenKind::Name) {
		advance();
		return token;
	}
	return std::nullopt;
}

TokenHeader TokenCursor::current() const {
	return it_->header;
}

TokenKind TokenCursor::current_kind() const {
	return it_->header.kind;
}

SourceOffset TokenCursor::current_offset() const {
	return it_->header.offset;
}

TokenHeader TokenCursor::peek() {
	skip_comments();
	return current();
}

TokenHeader TokenCursor::peek_ahead() {
	skip_comments();
	auto saved = it_;
	advance();
	auto token = peek();
	it_ = saved;
	return token;
}

TokenKind TokenCursor::peek_kind() {
	skip_comments();
	return it_->header.kind;
}

SourceOffset TokenCursor::peek_offset() {
	skip_comments();
	return it_->header.offset;
}

void TokenCursor::advance() {
	if (it_->header.kind != TokenKind::EndOfFile) {
		const auto header = it_->header;
		it_ += is_variable_length_token(header.kind) ? 2 : 1;
	}
}

void TokenCursor::skip_comments() {
	auto kind = it_->header.kind;
	while (kind == TokenKind::LineComment || kind == TokenKind::BlockComment) {
		it_ += 2;
		kind = it_->header.kind;
	}
}

std::string_view TokenCursor::get_lexeme(const SourceGuard& source) const {
	if (length == 0) {
		return get_fixed_length_lexeme(kind);
	} else {
		return source.get_text().substr(offset, length);
	}
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

std::string TokenCursor::to_message_string(const SourceGuard& source) const {
	auto format = get_token_message_format(kind);
	auto lexeme = get_lexeme(source);
	return fmt::vformat(format, fmt::make_format_args(lexeme));
}

std::string TokenCursor::to_log_string(const SourceGuard& source) const {
	auto token_kind = token_kind_to_string(kind);
	auto lexeme = get_lexeme(source);
	auto location = source.locate(offset);
	return fmt::format("{} `{}` {}", token_kind, lexeme, location.to_short_string());
}

} // namespace cero
