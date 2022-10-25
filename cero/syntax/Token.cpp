#include "Token.hpp"

bool is_variable_length_token(TokenKind kind)
{
	return get_token_length(kind) == 0xff;
}

std::string_view Token::get_lexeme(const Source& source) const
{
	return source.get_text().substr(offset, length);
}

std::string Token::to_string(const Source& source) const
{
	auto token_kind = magic_enum::enum_name(kind);

	if (is_variable_length_token(kind))
		return std::format("{}(`{}`)", token_kind, get_lexeme(source));

	return std::string(token_kind);
}
