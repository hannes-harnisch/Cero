#include "AstNode.hpp"

namespace cero {

std::string_view unary_operator_to_string(UnaryOperator op) {
	switch (op) {
		using enum UnaryOperator;
		case PreIncrement: return "prefix ++";
		case PreDecrement: return "prefix --";
		case PostIncrement: return "postfix ++";
		case PostDecrement: return "postfix --";
		case AddressOf: return "&";
		case Dereference: return "^";
		case Negate: return "-";
		case LogicalNot: return "!";
		case BitwiseNot: return "~";
	}
	fail_unreachable();
}

std::string_view binary_operator_to_string(BinaryOperator op) {
	switch (op) {
		using enum BinaryOperator;
		case Add: return "+";
		case Subtract: return "-";
		case Multiply: return "*";
		case Divide: return "/";
		case Remainder: return "%";
		case Power: return "**";
		case LogicalAnd: return "&&";
		case LogicalOr: return "||";
		case BitAnd: return "&";
		case BitOr: return "|";
		case Xor: return "~";
		case LeftShift: return "<<";
		case RightShift: return ">>";
		case Equal: return "==";
		case NotEqual: return "!=";
		case Less: return "<";
		case Greater: return ">";
		case LessEqual: return "<=";
		case GreaterEqual: return ">=";
		case Assign: return "=";
		case AddAssign: return "+=";
		case SubtractAssign: return "-=";
		case MultiplyAssign: return "*=";
		case DivideAssign: return "/=";
		case RemainderAssign: return "%=";
		case PowerAssign: return "**=";
		case AndAssign: return "&=";
		case OrAssign: return "|=";
		case XorAssign: return "~=";
		case LeftShiftAssign: return "<<=";
		case RightShiftAssign: return ">>=";
	}
	fail_unreachable();
}

} // namespace cero
