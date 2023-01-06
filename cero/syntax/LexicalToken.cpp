#include "LexicalToken.hpp"

#include <magic_enum.hpp>

namespace cero
{

namespace
{
	std::string_view get_token_message_format(Token kind)
	{
		using enum Token;
		switch (kind)
		{
			case Name: return "name `{}`";
			case NewLine: return "new line";
			case LineComment:
			case BlockComment: return "comment";
			case DecIntLiteral:
			case HexIntLiteral:
			case BinIntLiteral:
			case OctIntLiteral: return "integer literal `{}`";
			case FloatLiteral: return "floating-point literal `{}`";
			case CharLiteral: return "character literal {}";
			case StringLiteral: return "string literal {}";
			case EndOfFile: return "end of file";
			default: return "`{}`";
		}
	}
} // namespace

std::string_view LexicalToken::get_lexeme(const Source& source) const
{
	return source.get_text().substr(offset, length);
}

std::string LexicalToken::to_message_string(const Source& source) const
{
	return std::vformat(get_token_message_format(kind), std::make_format_args(get_lexeme(source)));
}

std::string LexicalToken::to_log_string(const Source& source) const
{
	auto token_kind = magic_enum::enum_name(kind);
	auto lexeme		= get_lexeme(source);
	if (kind == Token::NewLine)
		lexeme = "";

	auto location = locate_in(source);
	return std::format("{} `{}` [{}]", token_kind, lexeme, location.to_string());
}

SourceLocation LexicalToken::locate_in(const Source& source) const
{
	auto cursor = source.begin() + offset;
	return source.locate(cursor);
}

} // namespace cero