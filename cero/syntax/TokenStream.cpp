#include "TokenStream.hpp"

void TokenStream::append(Token token)
{
	tokens.emplace_back(token);
}

std::string TokenStream::to_string(const Source& source) const
{
	std::stringstream str;

	for (auto token : tokens)
	{
		str << token.to_debug_string(source);
		if (token.kind == TokenKind::NewLine)
			str << '\n';
		else
			str << ' ';
	}

	return str.str();
}

std::span<const Token> TokenStream::get_tokens() const
{
	return tokens;
}

Token TokenStream::at(uint32_t index) const
{
	return tokens.at(index);
}

void TokenStream::print(const Source& source) const
{
	std::clog << to_string(source);
}
