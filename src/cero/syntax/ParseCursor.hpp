#pragma once

#include "cero/syntax/TokenStream.hpp"

namespace cero {

class ParseCursor {
public:
	explicit ParseCursor(const TokenStream& token_stream);

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

	// Returns the token that was last advanced after.
	Token previous() const;

	// Moves cursor to the next token.
	void advance();

	// Advance to the next non-comment token.
	void skip_comments();

private:
	std::span<const Token>::iterator cursor;
};

} // namespace cero
