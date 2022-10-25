#pragma once

#include "syntax/Token.hpp"

#include <string_view>
#include <vector>

class TokenStream
{
	std::vector<Token> tokens;

public:
	void		append(Token token);
	std::string to_string(std::string_view source) const;
};
