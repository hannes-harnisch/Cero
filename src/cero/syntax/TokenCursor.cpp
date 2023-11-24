#include "TokenCursor.hpp"

namespace cero {

TokenCursor::TokenCursor(const TokenStream& token_stream) :
	it_(token_stream.stream_.begin()),
	end_(token_stream.stream_.end()) {
}

std::optional<Token> TokenCursor::next() {
	if (it_ != end_) {
		const auto header = it_->header;
		++it_;
		if (header.is_variable_length()) {
			const auto length = it_->length;
			++it_;
			return Token {header, length};
		} else {
			return Token {header, 0};
		}
	} else {
		return std::nullopt;
	}
}

bool TokenCursor::match(TokenKind kind) {
	auto token = peek();
	if (token.header.kind == kind) {
		advance();
		return true;
	}
	return false;
}

std::optional<Token> TokenCursor::match_token(TokenKind kind) {
	auto token = peek();
	if (token.header.kind == kind) {
		advance();
		return token;
	}
	return std::nullopt;
}

std::optional<Token> TokenCursor::match_name() {
	auto token = peek();
	if (token.header.kind == TokenKind::Name) {
		advance();
		return token;
	}
	return std::nullopt;
}

TokenKind TokenCursor::current_kind() const {
	return it_->header.kind;
}

Token TokenCursor::current() const {
	auto header = it_->header;

	uint32_t length;
	if (header.is_variable_length()) {
		length = it_[1].length;
	} else {
		length = 0;
	}
	return {header, length};
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

void TokenCursor::advance() {
	if (it_ != end_) {
		const auto header = it_->header;
		++it_;
		if (header.is_variable_length()) {
			++it_;
		}
	}
}

bool TokenCursor::valid() const {
	return it_ != end_;
}

void TokenCursor::skip_comments() {
	auto kind = it_->header.kind;
	while (kind == TokenKind::LineComment || kind == TokenKind::BlockComment) {
		advance();
		kind = it_->header.kind;
	}
}

} // namespace cero
