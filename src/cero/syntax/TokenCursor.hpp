#pragma once

#include "cero/syntax/TokenStream.hpp"

namespace cero {

/// Iterates over a token stream. An instance of this class must not outlive the token stream it is initialized with.
class TokenCursor {
public:
	/// Creates a cursor positioned at the first token of the given token stream.
	explicit TokenCursor(const TokenStream& token_stream);

	/// Returns the current token and then advances if not at the end, skipping comments.
	TokenHeader next();

	/// Returns true and advances if the current token kind equals the expected, skipping comments, otherwise returns false.
	bool match(TokenKind kind);

	/// Returns the current token and advances if the current token kind equals the expected, skipping comments, otherwise
	/// returns null.
	std::optional<TokenHeader> match_token(TokenKind kind);

	/// Returns the current token and advances if the current token kind is a name token, skipping comments, otherwise returns
	/// null.
	std::optional<TokenHeader> match_name();

	/// Returns the current token, skipping comments.
	TokenHeader peek();

	/// Returns the token after the current token, skipping comments.
	TokenHeader peek_ahead();

	/// Returns the current token kind, skipping comments.
	TokenKind peek_kind();

	/// Returns the current token offset, skipping comments.
	SourceOffset peek_offset();

	/// Returns the current token, without skipping comments.
	TokenHeader current() const;

	/// Returns the current token kind, without skipping comments.
	TokenKind current_kind() const;

	/// Returns the current token offset, without skipping comments.
	SourceOffset current_offset() const;

	/// Moves cursor to the next token.
	void advance();

	/// Advance to the next non-comment token.
	void skip_comments();

	std::string_view get_lexeme(const SourceGuard& source) const;
	std::string to_message_string(const SourceGuard& source) const;
	std::string to_log_string(const SourceGuard& source) const;

private:
	std::span<const TokenStream::Unit>::iterator it_;
};

} // namespace cero
