#pragma once

#include "cero/syntax/Token.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class TokenStream {
public:
	union Unit {
		TokenHeader header;
		uint32_t length;
	};

	uint32_t num_tokens() const;

	bool has_errors() const;

	std::span<const Unit> raw() const;

	std::string to_string(const LockedSource& source) const;

private:
	std::vector<Unit> stream_;
	uint32_t num_tokens_;
	bool has_errors_;

	explicit TokenStream(const LockedSource& source);

	void add_header(TokenKind kind, SourceOffset offset);
	void add_length(uint32_t length);

	friend class Lexer;
};

} // namespace cero
