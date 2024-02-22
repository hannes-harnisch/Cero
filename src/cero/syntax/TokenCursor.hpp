#pragma once

#include "cero/syntax/TokenStream.hpp"

namespace cero {

// An instance of this class must not outlive the token stream it is initialized with.
class TokenCursor {
public:
	explicit TokenCursor(const TokenStream& token_stream);

	// Returns the current token and then advances if not at the end.
	Token next();

	// Returns true and advances if the current token kind equals the expected, otherwise returns false.
	bool match(TokenKind kind);

	// Returns the current token and advances if the current token kind equals the expected, otherwise returns null.
	std::optional<Token> match_token(TokenKind kind);

	// Returns the current token and advances if the current token kind is a name token, otherwise returns null.
	std::optional<Token> match_name();

	// Returns the current token, skipping comments.
	Token peek();

	Token peek_ahead();

	// Returns the current token kind, skipping comments.
	TokenKind peek_kind();

	// Returns the current token, not skipping comments.
	Token current() const;

	// Returns the current token kind, not skipping comments.
	TokenKind current_kind() const;

	// Moves cursor to the next token.
	void advance();

	// Advance to the next non-comment token.
	void skip_comments();

private:
	std::span<const TokenStream::Unit>::iterator it_;
};

} // namespace cero
