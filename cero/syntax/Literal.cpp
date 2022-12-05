#include "Literal.hpp"

NumericLiteral evaluate_dec_int_literal(std::string_view)
{
	return {Literal::Decimal};
}

NumericLiteral evaluate_hex_int_literal(std::string_view)
{
	return {Literal::Hexadecimal};
}

NumericLiteral evaluate_bin_int_literal(std::string_view)
{
	return {Literal::Binary};
}

NumericLiteral evaluate_oct_int_literal(std::string_view)
{
	return {Literal::Octal};
}

NumericLiteral evaluate_float_literal(std::string_view)
{
	return {Literal::Float};
}

NumericLiteral evaluate_char_literal(std::string_view)
{
	return {Literal::Character};
}

StringLiteral::StringLiteral(std::string_view lexeme) :
	value(lexeme) // TODO: escape sequences
{}
