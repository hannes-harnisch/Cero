#include "ParseCursor.hpp"

namespace cero {

ParseCursor::ParseCursor(const TokenStream& token_stream) :
	cursor(token_stream.get_tokens().begin()) {
}

bool ParseCursor::match(TokenKind kind) {
	auto token = peek();
	if (token.header.kind == kind) {
		advance();
		return true;
	}
	return false;
}

std::optional<Token> ParseCursor::match_token(TokenKind kind) {
	auto token = peek();
	if (token.header.kind == kind) {
		advance();
		return token;
	}
	return std::nullopt;
}

std::optional<Token> ParseCursor::match_name() {
	auto token = peek();
	if (token.header.kind == TokenKind::Name) {
		advance();
		return token;
	}
	return std::nullopt;
}

TokenKind ParseCursor::current_kind() const {
	return cursor->header.kind;
}

Token ParseCursor::current() const {
	return *cursor;
}

Token ParseCursor::previous() const {
	return cursor[-1];
}

Token ParseCursor::peek() {
	skip_comments();
	return *cursor;
}

Token ParseCursor::peek_ahead() {
	skip_comments();
	auto saved = cursor;
	advance();
	auto token = peek();
	cursor = saved;
	return token;
}

TokenKind ParseCursor::peek_kind() {
	skip_comments();
	return cursor->header.kind;
}

void ParseCursor::advance() {
	if (cursor->header.kind != TokenKind::EndOfFile) {
		++cursor;
	}
}

void ParseCursor::skip_comments() {
	auto kind = cursor->header.kind;
	while (kind == TokenKind::LineComment || kind == TokenKind::BlockComment) {
		++cursor;
		kind = cursor->header.kind;
	}
}

} // namespace cero
