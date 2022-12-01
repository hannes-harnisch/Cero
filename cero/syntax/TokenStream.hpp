#pragma once

#include "syntax/Token.hpp"

#include <span>
#include <string>
#include <vector>

class TokenStream
{
	std::vector<Token> tokens;

public:
	void				   append(Token token);
	std::string			   to_string(const Source& source) const;
	std::span<const Token> get_tokens() const;
	Token				   at(uint32_t index) const;
	void				   print(const Source& source) const;
};
