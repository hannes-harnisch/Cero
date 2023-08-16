#include "ParseCursor.hpp"

namespace cero {

ParseCursor::ParseCursor(const TokenStream& token_stream) :
	cursor(token_stream.get_tokens().begin()) {
}

bool ParseCursor::match(Token kind) {
	auto token = next();
	if (token.kind == kind) {
		advance();
		return true;
	}
	return false;
}

std::optional<LexicalToken> ParseCursor::match_token(Token kind) {
	auto token = next();
	if (token.kind == kind) {
		advance();
		return token;
	}
	return std::nullopt;
}

std::optional<LexicalToken> ParseCursor::match_name() {
	auto token = next();
	if (token.kind == Token::Name) {
		advance();
		return token;
	}
	return std::nullopt;
}

Token ParseCursor::peek_kind() const {
	return cursor->kind;
}

LexicalToken ParseCursor::peek() const {
	return *cursor;
}

LexicalToken ParseCursor::previous() const {
	return cursor[-1];
}

LexicalToken ParseCursor::next() {
	auto kind = cursor->kind;
	while (kind == Token::LineComment || kind == Token::BlockComment) {
		advance();
		kind = cursor->kind;
	}
	return *cursor;
}

Token ParseCursor::next_kind() {
	auto kind = cursor->kind;
	while (kind == Token::LineComment || kind == Token::BlockComment) {
		advance();
		kind = cursor->kind;
	}
	return kind;
}

void ParseCursor::advance() {
	if (cursor->kind != Token::EndOfFile)
		++cursor;
}

} // namespace cero
