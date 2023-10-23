#include "TokenStream.hpp"

namespace cero {

std::span<const Token> TokenStream::get_tokens() const {
	return tokens;
}

Token TokenStream::at(size_t index) const {
	return tokens.at(index);
}

std::string TokenStream::to_string(const Source& source) const {
	std::stringstream str;

	for (auto token : tokens)
		str << token.to_log_string(source) << '\n';

	return std::move(str).str();
}

void TokenStream::log(const Source& source) const {
	std::cout << to_string(source);
}

} // namespace cero
