#pragma once

#include "syntax/TokenStream.hpp"

namespace cero {

class ParseCursor {
public:
	explicit ParseCursor(const TokenStream& token_stream);

	bool match(Token kind);

	std::optional<LexicalToken> match_token(Token kind);
	std::optional<LexicalToken> match_name();

	Token		 peek_kind() const;
	LexicalToken peek() const;
	LexicalToken previous() const;

	LexicalToken next();
	Token		 next_kind();
	void		 advance();

private:
	std::span<const LexicalToken>::iterator cursor;
};

} // namespace cero
