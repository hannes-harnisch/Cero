#pragma once

#include "cero/syntax/TokenStream.hpp"

namespace cero
{

class ParseCursor
{
	TokenStream::Iterator cursor;

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
};

} // namespace cero
