#include "ParseCursor.hpp"

namespace cero
{

ParseCursor::ParseCursor(cero::TokenStream::Iterator cursor) :
	cursor(cursor)
{}

Token ParseCursor::peek_kind() const
{
	return cursor->kind;
}

LexicalToken ParseCursor::peek() const
{
	return *cursor;
}

LexicalToken ParseCursor::previous() const
{
	return cursor[-1];
}

std::optional<LexicalToken> ParseCursor::match(Token kind)
{
	auto token = next_breakable();
	if (token.kind == kind)
	{
		advance();
		return token;
	}
	return {};
}

LexicalToken ParseCursor::next_breakable()
{
	auto kind = cursor->kind;
	while (kind == Token::NewLine || kind == Token::LineComment || kind == Token::BlockComment)
	{
		advance();
		kind = cursor->kind;
	}
	return *cursor;
}

bool ParseCursor::is_next_new_line()
{
	auto kind = cursor->kind;
	while (kind == Token::LineComment || kind == Token::BlockComment)
	{
		advance();
		kind = cursor->kind;
	}
	return kind == Token::NewLine;
}

void ParseCursor::advance()
{
	++cursor;
}

void ParseCursor::retreat_to_last_breakable()
{
	Token kind;
	do
	{
		--cursor;
		kind = cursor->kind;
	}
	while (kind == Token::NewLine || kind == Token::LineComment || kind == Token::BlockComment);
}

} // namespace cero
