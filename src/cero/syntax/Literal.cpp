#include "Literal.hpp"

namespace cero {

AstNumericLiteralExpr evaluate_dec_int_literal(std::string_view) {
	return {{0}, NumericLiteralKind::Decimal};
}

AstNumericLiteralExpr evaluate_hex_int_literal(std::string_view) {
	return {{0}, NumericLiteralKind::Hexadecimal};
}

AstNumericLiteralExpr evaluate_bin_int_literal(std::string_view) {
	return {{0}, NumericLiteralKind::Binary};
}

AstNumericLiteralExpr evaluate_oct_int_literal(std::string_view) {
	return {{0}, NumericLiteralKind::Octal};
}

AstNumericLiteralExpr evaluate_float_literal(std::string_view) {
	return {{0}, NumericLiteralKind::Float};
}

AstNumericLiteralExpr evaluate_char_literal(std::string_view) {
	return {{0}, NumericLiteralKind::Character};
}

AstStringLiteralExpr evaluate_string_literal(std::string_view lexeme) {
	return {{0}, std::string(lexeme)}; // TODO: escape sequences
}

} // namespace cero
