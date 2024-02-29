#include "TokenCursor.hpp"

namespace cero {

TokenCursor::TokenCursor(const TokenStream& token_stream) :
	it_(token_stream.raw().begin()) {
}

Token TokenCursor::next() {
	const auto header = it_->header;

	if (header.kind != TokenKind::EndOfFile) {
		++it_;
		if (header.is_variable_length()) {
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
	if (header.is_variable_length()) {
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
		if (header.is_variable_length()) {
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
