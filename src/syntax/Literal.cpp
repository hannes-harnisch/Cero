#include "Literal.hpp"

namespace cero {

AstNumericLiteralExpr evaluate_dec_int_literal(std::string_view) {
	return {NumericLiteralKind::Decimal};
}

AstNumericLiteralExpr evaluate_hex_int_literal(std::string_view) {
	return {NumericLiteralKind::Hexadecimal};
}

AstNumericLiteralExpr evaluate_bin_int_literal(std::string_view) {
	return {NumericLiteralKind::Binary};
}

AstNumericLiteralExpr evaluate_oct_int_literal(std::string_view) {
	return {NumericLiteralKind::Octal};
}

AstNumericLiteralExpr evaluate_float_literal(std::string_view) {
	return {NumericLiteralKind::Float};
}

AstNumericLiteralExpr evaluate_char_literal(std::string_view) {
	return {NumericLiteralKind::Character};
}

AstStringLiteralExpr evaluate_string_literal(std::string_view lexeme) {
	return {std::string(lexeme)}; // TODO: escape sequences
}

} // namespace cero