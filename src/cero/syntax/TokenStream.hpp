#pragma once

#include "cero/syntax/Token.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class TokenStream {
public:
	/// Number of tokens in the stream.
	uint32_t num_tokens() const;

	/// Whether syntax errors were encountered during lexing.
	bool has_errors() const;

	/// Get a view of the underlying array of tokens.
	std::span<const Token> raw() const;

	/// Creates a list-like string representation of the token stream.
	std::string to_string(const SourceGuard& source) const;

private:
	std::vector<Token> stream_;
	bool has_errors_ = false;

	/// Reserves storage for the token stream based on the length of the source code input.
	explicit TokenStream(const SourceGuard& source);

	/// Appends a token to the stream.
	void add_token(TokenKind kind, SourceOffset offset);

	friend class Lexer;
};

} // namespace cero
