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

std::string TokenStream::to_string(const SourceGuard& source) const {
	auto str = fmt::format("Token stream for {} ({} token{})\n", source.get_name(), num_tokens_, num_tokens_ == 1 ? "" : "s");

	TokenCursor cursor(*this);
	Token token;
	do {
		token = cursor.next();

		// This loop is technically quadratic since Token::to_log_string will search the source for the code location
		// repeatedly, but it really doesn't matter.
		str += fmt::format("\t{}\n", token.to_log_string(source));
	} while (token.kind != TokenKind::EndOfFile);

	return str;
}

TokenStream::TokenStream(const SourceGuard& source) :
	num_tokens_(0),
	has_errors_(false) {
	stream_.reserve(source.get_length()); // TODO: find heuristic for this
}

void TokenStream::add_header(TokenKind kind, SourceOffset offset) {
	stream_.emplace_back(TokenHeader {kind, offset & 0x00ffffffu});
	++num_tokens_;
}

void TokenStream::add_length(uint32_t length) {
	stream_.emplace_back(Unit {.length = length});
}

} // namespace cero
