#pragma once

#include "syntax/LexicalToken.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class TokenStream {
public:
	std::span<const LexicalToken> get_tokens() const;
	LexicalToken at(size_t index) const;

	std::string to_string(const Source& source) const;
	void log(const Source& source) const;

private:
	std::vector<LexicalToken> tokens;

	friend class Lexer;
};

} // namespace cero
