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
	std::span<const LexicalToken> get_tokens() const;
	LexicalToken				  at(uint32_t index) const;
	std::string					  to_log_string(const Source& source) const;
	void						  log(const Source& source) const;
};
