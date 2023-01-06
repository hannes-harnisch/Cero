#pragma once

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

NumericLiteral evaluate_dec_int_literal(std::string_view lexeme);
NumericLiteral evaluate_hex_int_literal(std::string_view lexeme);
NumericLiteral evaluate_bin_int_literal(std::string_view lexeme);
NumericLiteral evaluate_oct_int_literal(std::string_view lexeme);
NumericLiteral evaluate_float_literal(std::string_view lexeme);
NumericLiteral evaluate_char_literal(std::string_view lexeme);

struct StringLiteral
{
	std::string value;

	explicit StringLiteral(std::string_view lexeme);

	bool operator==(const StringLiteral&) const = default;
};
