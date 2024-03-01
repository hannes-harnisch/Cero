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
			case AmpersandAmpersand:
			case PipePipe:
			case EqualsEquals:
			case BangEquals:
			case LeftAngleEquals:
			case RightAngleEquals:
			case StarStar:
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

Token TokenCursor::next() {
	const auto header = it_->header;

	if (header.kind != TokenKind::EndOfFile) {
		++it_;
		if (is_variable_length_token(header.kind)) {
			const auto length = it_->length;
			++it_;
			return Token {header, length};
		}
	}

	return Token {header, 0};
}

bool TokenCursor::match(TokenKind kind) {
	auto token = peek();
	if (token.kind == kind) {
		advance();
		return true;
	}
	return false;
}

std::optional<Token> TokenCursor::match_token(TokenKind kind) {
	auto token = peek();
	if (token.kind == kind) {
		advance();
		return token;
	}
	return std::nullopt;
}

std::optional<Token> TokenCursor::match_name() {
	auto token = peek();
	if (token.kind == TokenKind::Name) {
		advance();
		return token;
	}
	return std::nullopt;
}

Token TokenCursor::current() const {
	auto header = it_->header;
	if (is_variable_length_token(header.kind)) {
		return {header, it_[1].length};
	} else {
		return {header, 0};
	}
}

TokenKind TokenCursor::current_kind() const {
	return it_->header.kind;
}

SourceOffset TokenCursor::current_offset() const {
	return it_->header.offset;
}

Token TokenCursor::peek() {
	skip_comments();
	return current();
}

Token TokenCursor::peek_ahead() {
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
		++it_;
		if (is_variable_length_token(header.kind)) {
			++it_;
		}
	}
}

void TokenCursor::skip_comments() {
	auto kind = it_->header.kind;
	while (kind == TokenKind::LineComment || kind == TokenKind::BlockComment) {
		it_ += 2;
		kind = it_->header.kind;
	}
}

} // namespace cero
