#pragma once

#include <string_view>

namespace cero {

void evaluate_dec_int_literal(std::string_view lexeme);
void evaluate_hex_int_literal(std::string_view lexeme);
void evaluate_bin_int_literal(std::string_view lexeme);
void evaluate_oct_int_literal(std::string_view lexeme);
void evaluate_float_literal(std::string_view lexeme);
void evaluate_char_literal(std::string_view lexeme);
std::string evaluate_string_literal(std::string_view lexeme);

} // namespace cero
