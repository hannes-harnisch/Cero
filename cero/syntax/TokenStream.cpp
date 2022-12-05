#include "TokenStream.hpp"

void TokenStream::append(LexicalToken token)
{
	tokens.emplace_back(token);
}

std::string TokenStream::to_string(const Source& source) const
{
	std::stringstream str;

	for (auto token : tokens)
	{
		str << token.to_debug_string(source);
		if (token.kind == Token::NewLine)
			str << '\n';
		else
			str << ' ';
	}

	return str.str();
}

std::span<const LexicalToken> TokenStream::get_tokens() const
{
	return tokens;
}

LexicalToken TokenStream::at(uint32_t index) const
{
	return tokens.at(index);
}

void TokenStream::print(const Source& source) const
{
	std::clog << to_string(source);
}
