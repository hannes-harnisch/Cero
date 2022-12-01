#include "Token.hpp"

#include "util/Fail.hpp"

#include <magic_enum.hpp>

namespace
{
	bool is_variable_length_token(TokenKind t)
	{
		using enum TokenKind;
		return t == Name || t == LineComment || t == BlockComment || t == DecIntLiteral || t == HexIntLiteral
			   || t == BinIntLiteral || t == OctIntLiteral || t == DecFloatLiteral || t == HexFloatLiteral || t == CharLiteral
			   || t == StringLiteral;
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

std::string_view Token::get_lexeme(const Source& source) const
{
	return source.get_text().substr(offset, length);
}

std::string Token::to_message_string(const Source& source) const
{
	return std::vformat(get_token_message_format(kind), std::make_format_args(get_lexeme(source)));
}

std::string Token::to_debug_string(const Source& source) const
{
	auto token_kind = magic_enum::enum_name(kind);

	if (is_variable_length_token(kind))
		return std::format("{}(`{}`)", token_kind, get_lexeme(source));

	return std::string(token_kind);
}

SourceLocation Token::locate_in(const Source& source) const
{
	auto cursor = source.begin() + offset;
	return source.locate(cursor);
}
