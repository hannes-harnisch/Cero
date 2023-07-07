#pragma once

#include "syntax/AstNode.hpp"

#include <string_view>

namespace cero {

AstNumericLiteralExpr evaluate_dec_int_literal(std::string_view lexeme);
AstNumericLiteralExpr evaluate_hex_int_literal(std::string_view lexeme);
AstNumericLiteralExpr evaluate_bin_int_literal(std::string_view lexeme);
AstNumericLiteralExpr evaluate_oct_int_literal(std::string_view lexeme);
AstNumericLiteralExpr evaluate_float_literal(std::string_view lexeme);
AstNumericLiteralExpr evaluate_char_literal(std::string_view lexeme);
AstStringLiteralExpr  evaluate_string_literal(std::string_view lexeme);

} // namespace cero
