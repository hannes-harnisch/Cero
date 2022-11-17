#include "Token.hpp"

#include "util/Fail.hpp"

#include <magic_enum.hpp>

namespace
{
	bool is_variable_length_token(TokenKind k)
	{
		using enum TokenKind;
		return k == Name || k == LineComment || k == BlockComment || k == DecIntLiteral || k == HexIntLiteral
			   || k == BinIntLiteral || k == OctIntLiteral || k == DecFloatLiteral || k == HexFloatLiteral || k == CharLiteral
			   || k == StringLiteral;
	}

	std::string_view get_token_message_format(TokenKind kind)
	{
		using enum TokenKind;
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
			case DecFloatLiteral:
			case HexFloatLiteral: return "floating-point literal `{}`";
			case CharLiteral: return "character literal {}";
			case StringLiteral: return "string literal {}";
			case EndOfFile: return "end of file";
		}
		return "`{}`";
	}
} // namespace

std::string_view Token::get_lexeme_from(const Source& source) const
{
	return source.get_text().substr(offset, length);
}

std::string Token::describe_for_message(const Source& source) const
{
	return std::vformat(get_token_message_format(kind), std::make_format_args(get_lexeme_from(source)));
}

std::string Token::to_string(const Source& source) const
{
	auto token_kind = magic_enum::enum_name(kind);

	if (is_variable_length_token(kind))
		return std::format("{}(`{}`)", token_kind, get_lexeme_from(source));

	return std::string(token_kind);
}

SourceLocation Token::locate_in(const Source& source) const
{
	auto cursor = source.begin() + offset;
	return source.locate(cursor);
}
