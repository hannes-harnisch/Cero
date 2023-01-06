#pragma once

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

		bool operator==(const NumericLiteral&) const = default;
	};

	struct StringLiteral
	{
		std::string value;

		explicit StringLiteral(std::string_view lexeme);

		bool operator==(const StringLiteral&) const = default;
	};
} // namespace ast

ast::NumericLiteral evaluate_dec_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_hex_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_bin_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_oct_int_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_float_literal(std::string_view lexeme);
ast::NumericLiteral evaluate_char_literal(std::string_view lexeme);

} // namespace cero
