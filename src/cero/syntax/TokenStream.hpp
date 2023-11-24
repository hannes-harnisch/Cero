#pragma once

#include "cero/syntax/Token.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class TokenStream {
public:
	uint32_t num_tokens() const;

	std::string to_string(const SourceLock& source) const;

private:
	union Unit {
		TokenHeader header;
		uint32_t length;
	};

	std::vector<Unit> stream_;
	uint32_t num_tokens_;

	TokenStream();

	void add_header(TokenHeader header);
	void add_length(uint32_t length);

	friend class Lexer;
	friend class TokenCursor;
};

} // namespace cero
