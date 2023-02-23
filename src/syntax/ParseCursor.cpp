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
	auto token = next();
	if (token.kind == kind)
	{
		advance();
		return token;
	}
	return {};
}

LexicalToken ParseCursor::next()
{
	auto kind = cursor->kind;
	while (kind == Token::LineComment || kind == Token::BlockComment)
	{
		advance();
		kind = cursor->kind;
	}
	return *cursor;
}

Token ParseCursor::next_kind()
{
	auto kind = cursor->kind;
	while (kind == Token::LineComment || kind == Token::BlockComment)
	{
		advance();
		kind = cursor->kind;
	}
	return kind;
}

void ParseCursor::advance()
{
	++cursor;
}

void ParseCursor::retreat()
{
	Token kind;
	do
	{
		--cursor;
		kind = cursor->kind;
	}
	while (kind == Token::LineComment || kind == Token::BlockComment);
}

} // namespace cero
