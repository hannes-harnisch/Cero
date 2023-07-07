#pragma once

#include "syntax/TokenStream.hpp"

namespace cero {

class ParseCursor {
public:
	explicit ParseCursor(const TokenStream& token_stream);

	Token		 peek_kind() const;
	LexicalToken peek() const;
	LexicalToken previous() const;

	std::optional<LexicalToken> match(Token kind);

	LexicalToken next();
	Token		 next_kind();
	void		 advance();
	void		 retreat();

private:
	std::span<const LexicalToken>::iterator cursor;
};

} // namespace cero
