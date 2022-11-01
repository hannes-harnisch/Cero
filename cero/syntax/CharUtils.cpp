#include "CharUtils.hpp"

char to_upper_ascii(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - 32;

	return c;
}

char to_lower_ascii(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + 32;

	return c;
}

bool is_breaking_whitespace(char c)
{
	return c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

bool is_non_breaking_whitespace(char c)
{
	return c == ' ' || c == '\t';
}

bool is_word_char(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || is_dec_digit(c) || c == '_';
}

bool can_begin_names(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_dec_digit(char c)
{
	return c >= '0' && c <= '9';
}

bool is_hex_digit(char c)
{
	return is_dec_digit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

bool is_oct_digit(char c)
{
	return c >= '0' && c <= '7';
}

bool is_bin_digit(char c)
{
	return c == '0' || c == '1';
}
