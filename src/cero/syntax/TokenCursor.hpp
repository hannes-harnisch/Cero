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

	/// Returns the current token and then advances if not at the end.
	Token next() {
		const auto token = *it_;
		if (token.kind != TokenKind::EndOfFile) {
			++it_;
		}
		return token;
	}

	/// Returns true and advances if the current token kind equals the expected, skipping comments, otherwise returns false.
	bool match(TokenKind kind) {
		auto token = peek();
		if (token.kind == kind) {
			advance();
			return true;
		}
		return false;
	}

	/// Returns the current token and advances if the current token kind equals the expected, skipping comments, otherwise
	/// returns null.
	std::optional<Token> match_token(TokenKind kind) {
		auto token = peek();
		if (token.kind == kind) {
			advance();
			return token;
		}
		return std::nullopt;
	}

	/// Returns a string view of the lexeme and advances if the current token kind is an identifier token, skipping comments,
	/// otherwise returns an empty string.
	std::string_view match_name(const SourceGuard& source) {
		if (peek_kind() == TokenKind::Name) {
			auto identifier = get_lexeme(source);
			advance();
			return identifier;
		}
		return {};
	}

	/// Returns the current token, skipping comments.
	Token peek() {
		skip_comments();
		return current();
	}

	/// Returns the token after the current token, skipping comments.
	Token peek_ahead() {
		skip_comments();
		auto saved = it_;
		advance();
		auto token = peek();
		it_ = saved;
		return token;
	}

	/// Returns the current token kind, skipping comments.
	TokenKind peek_kind() {
		skip_comments();
		return it_->kind;
	}

	/// Returns the current token offset, skipping comments.
	SourceOffset peek_offset() {
		skip_comments();
		return it_->offset;
	}

	/// Returns the current token, without skipping comments.
	Token current() const {
		return *it_;
	}

	/// Returns the current token kind, without skipping comments.
	TokenKind current_kind() const {
		return it_->kind;
	}

	/// Returns the current token offset, without skipping comments.
	SourceOffset current_offset() const {
		return it_->offset;
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
