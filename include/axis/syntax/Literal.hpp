#pragma once

#include <string>
#include <string_view>

namespace cero
{

namespace ast
{
	enum class Literal : uint8_t
	{
		Decimal,
		Hexadecimal,
		Binary,
		Octal,
		Float,
		Character
	};

	struct NumericLiteral
	{
		Literal kind = {};
		// TODO: multiprecision value
	};

	struct StringLiteral
	{
		std::string value;
	};
} // namespace ast

ast::NumericLiteral evaluate_dec_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_hex_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_bin_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_oct_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_float_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_char_literal(std::string_view lexeme);
ast::StringLiteral	evaluate_string_literal(std::string_view lexeme);

} // namespace cero
