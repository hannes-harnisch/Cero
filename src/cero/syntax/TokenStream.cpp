#include "TokenStream.hpp"

#include "cero/syntax/TokenCursor.hpp"

namespace cero {

uint32_t TokenStream::num_tokens() const {
	return num_tokens_;
}

bool TokenStream::has_errors() const {
	return has_errors_;
}

std::span<const TokenStream::Unit> TokenStream::raw() const {
	return {stream_};
}

std::string TokenStream::to_string(const SourceLock& source) const {
	auto str = std::format("Token stream for {} ({} token{})\n", source.get_path(), num_tokens_, num_tokens_ == 1 ? "" : "s");

	TokenCursor cursor(*this);
	while (auto next = cursor.next()) {
		auto token = *next;

		auto lexeme = token.get_lexeme(source);
		auto kind_str = token_kind_to_string(token.header.kind);
		auto location = source.locate(token.header.offset); // yes, this makes this algorithm quadratic, but it's not important
		str += std::format("\t{} `{}` [{}:{}]\n", kind_str, lexeme, location.line, location.column);
	}

	return str;
}

TokenStream::TokenStream(const SourceLock& source) :
	num_tokens_(0),
	has_errors_(false) {
	stream_.reserve(source.get_length()); // TODO: find heuristic for this
}

void TokenStream::add_header(TokenHeader header) {
	stream_.emplace_back(header);
	++num_tokens_;
}

void TokenStream::add_length(uint32_t length) {
	stream_.emplace_back(Unit {.length = length});
}

} // namespace cero
