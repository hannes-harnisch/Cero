#pragma once

#include "syntax/TokenStream.hpp"

namespace cero
{

class ParseCursor
{
	TokenStream::Iterator cursor;

public:
	explicit ParseCursor(TokenStream::Iterator cursor);

	Token		 peek_kind() const;
	LexicalToken peek() const;
	LexicalToken previous() const;

	std::optional<LexicalToken> match(Token kind);

	LexicalToken next_breakable();
	bool		 is_next_new_line();
	void		 advance();
	void		 retreat();
	void		 skip_enclosed_sequence(Token left, Token right);
};

} // namespace cero