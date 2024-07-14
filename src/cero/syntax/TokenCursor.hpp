#pragma once

#include "cero/syntax/Encoding.hpp"
#include "cero/syntax/TokenStream.hpp"

namespace cero {

/// Iterates over a token stream. An instance of this class must not outlive the token stream it is initialized with.
class TokenCursor {
public:
	/// Creates a cursor positioned at the first token of the given token stream.
	explicit TokenCursor(const TokenStream& token_stream) :
		it_(token_stream.raw().begin()) {
	}

	/// Returns the current token.
	Token peek() const {
		return *it_;
	}

	/// Returns the current token kind.
	TokenKind peek_kind() const {
		return it_->kind;
	}

	/// Returns the current token offset.
	SourceOffset peek_offset() const {
		return it_->offset;
	}

	/// Returns the current token and then advances if not at the end.
	Token next() {
		const auto token = *it_;
		advance();
		return token;
	}

	/// Returns true and advances if the current token kind equals the expected, otherwise returns false.
	bool match(TokenKind kind) {
		if (it_->kind == kind) {
			advance();
			return true;
		}
		return false;
	}

	/// Returns the current token and advances if the current token kind equals the expected, otherwise returns null.
	std::optional<Token> match_token(TokenKind kind) {
		auto token = *it_;
		if (token.kind == kind) {
			advance();
			return token;
		}
		return std::nullopt;
	}

	/// Returns a string view of the lexeme and advances if the current token kind is an identifier token, otherwise returns an
	/// empty string.
	std::string_view match_name(const SourceGuard& source) {
		if (it_->kind == TokenKind::Name) {
			auto identifier = get_lexeme(source);
			advance();
			return identifier;
		}
		return {};
	}

	/// Returns the token after the current token.
	Token peek_ahead() {
		auto saved = it_;
		advance();
		auto token = *it_;
		it_ = saved;
		return token;
	}

	std::string_view get_lexeme(const SourceGuard& source) const {
		const auto offset = it_->offset;
		const auto length = it_->kind != TokenKind::EndOfFile ? it_[1].offset - offset : 0;

		auto lexeme = source.get_text().substr(offset, length);

		size_t whitespace = 0;
		const auto end = lexeme.rend();
		for (auto it = lexeme.rbegin(); it != end; ++it) {
			if (is_whitespace(*it)) {
				++whitespace;
			} else {
				break;
			}
		}
		lexeme.remove_suffix(whitespace);

		return lexeme;
	}

	/// Moves cursor to the next token.
	void advance() {
		if (it_->kind != TokenKind::EndOfFile) {
			++it_;
		}
	}

	/// Advance to the next non-comment token.
	void skip_comments() {
		auto kind = it_->kind;
		while (kind == TokenKind::LineComment || kind == TokenKind::BlockComment) {
			++it_;
			kind = it_->kind;
		}
	}

private:
	std::span<const Token>::iterator it_;
};

} // namespace cero
