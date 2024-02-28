#include "Literal.hpp"

namespace cero {

void evaluate_dec_int_literal(std::string_view) {
}

void evaluate_hex_int_literal(std::string_view) {
}

void evaluate_bin_int_literal(std::string_view) {
}

void evaluate_oct_int_literal(std::string_view) {
}

void evaluate_float_literal(std::string_view) {
}

void evaluate_char_literal(std::string_view) {
}

std::string evaluate_string_literal(std::string_view lexeme) {
	// TODO: escape sequences
	return std::string(lexeme);
}

} // namespace cero
