#include "Literal.hpp"

namespace cero
{

ast::NumericLiteral evaluate_dec_int_literal(std::string_view)
{
	return {ast::Literal::Decimal};
}

ast::NumericLiteral evaluate_hex_int_literal(std::string_view)
{
	return {ast::Literal::Hexadecimal};
}

ast::NumericLiteral evaluate_bin_int_literal(std::string_view)
{
	return {ast::Literal::Binary};
}

ast::NumericLiteral evaluate_oct_int_literal(std::string_view)
{
	return {ast::Literal::Octal};
}

ast::NumericLiteral evaluate_float_literal(std::string_view)
{
	return {ast::Literal::Float};
}

ast::NumericLiteral evaluate_char_literal(std::string_view)
{
	return {ast::Literal::Character};
}

ast::StringLiteral evaluate_string_literal(std::string_view lexeme)
{
	return {std::string(lexeme)}; // TODO: escape sequences
}

} // namespace cero