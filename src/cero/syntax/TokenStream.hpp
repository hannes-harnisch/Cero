#pragma once

#include "cero/syntax/Token.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class TokenStream {
public:
	uint32_t num_tokens() const;

	bool has_errors() const;

	std::span<const Token> raw() const;

	std::string to_string(const SourceGuard& source) const;

private:
	std::vector<Token> stream_;
	bool has_errors_;

	explicit TokenStream(const SourceGuard& source);

	void add_token(TokenKind kind, SourceOffset offset);

	friend class Lexer;
};

} // namespace cero
