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

void ParseCursor::retreat()
{
	--cursor;
}

void ParseCursor::skip_enclosed_sequence(Token left, Token right)
{
	uint32_t unclosed = 1;
	auto	 token	  = next_breakable().kind;
	while (token != Token::EndOfFile)
	{
		if (token == left)
			++unclosed;
		else if (token == right && --unclosed == 0)
			break;

		token = next_breakable().kind;
	}
	advance();
}

} // namespace cero