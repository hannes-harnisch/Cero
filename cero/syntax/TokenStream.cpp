#include "TokenStream.hpp"

void TokenStream::append(LexicalToken token)
{
	tokens.emplace_back(token);
}

std::span<const LexicalToken> TokenStream::get_tokens() const
{
	return tokens;
}

LexicalToken TokenStream::at(uint32_t index) const
{
	return tokens.at(index);
}

std::string TokenStream::to_string(const Source& source) const
{
	std::stringstream str;

	for (auto token : tokens)
		str << token.to_log_string(source) << '\n';

	return str.str();
}

void TokenStream::log(const Source& source) const
{
	std::clog << to_string(source);
}
