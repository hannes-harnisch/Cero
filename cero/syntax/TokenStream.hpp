#pragma once

#include "syntax/LexicalToken.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero
{

class TokenStream
{
	std::vector<LexicalToken> tokens;

public:
	using Iterator = std::vector<LexicalToken>::const_iterator;

	explicit TokenStream(std::vector<LexicalToken> tokens);

	std::span<const LexicalToken> get_tokens() const;
	LexicalToken				  at(size_t index) const;
	Iterator					  begin() const;
	Iterator					  end() const;

	std::string to_string(const Source& source) const;
	void		log(const Source& source) const;
};

} // namespace cero
