#include "TokenStream.hpp"

void TokenStream::append(Token token)
{
	tokens.emplace_back(token);
}

std::string TokenStream::to_string(std::string_view source) const
{
	constexpr unsigned AVG_TOKEN_LENGTH = 5; // arbitrary heuristic

	std::string str;
	str.reserve(tokens.size() * AVG_TOKEN_LENGTH);

	for (auto token : tokens)
		str += std::format("`{}` ", token.to_string(source));

	return str;
}
