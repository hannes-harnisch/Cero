#pragma once

#include "syntax/LexicalToken.hpp"

#include <span>
#include <string>
#include <vector>

class TokenStream
{
	std::vector<LexicalToken> tokens;

public:
	void						  append(LexicalToken token);
	std::string					  to_string(const Source& source) const;
	std::span<const LexicalToken> get_tokens() const;
	LexicalToken				  at(uint32_t index) const;
	void						  print(const Source& source) const;
};
