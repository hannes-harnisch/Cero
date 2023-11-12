#pragma once

#include "cero/syntax/Token.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class TokenStream {
public:
	std::span<const Token> get_tokens() const;
	Token at(size_t index) const;

	std::string to_string(const SourceLock& source) const;

private:
	std::vector<Token> tokens;

	friend class Lexer;
};

} // namespace cero
