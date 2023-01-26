#include "cero/syntax/TokenStream.hpp"

namespace cero
{

std::span<const LexicalToken> TokenStream::get_tokens() const
{
	return tokens;
}

LexicalToken TokenStream::at(size_t index) const
{
	return tokens.at(index);
}

std::string TokenStream::to_string(const Source& source) const
{
	std::stringstream str;

	for (auto token : tokens)
		str << token.to_log_string(source) << '\n';

	return std::move(str).str();
}

void TokenStream::log(const Source& source) const
{
	std::clog << to_string(source);
}

TokenStream::Iterator TokenStream::begin() const
{
	return tokens.begin();
}

TokenStream::Iterator TokenStream::end() const
{
	return tokens.end();
}

} // namespace cero