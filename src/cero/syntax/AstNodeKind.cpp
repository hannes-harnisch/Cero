#include "AstNodeKind.hpp"

#include "cero/util/Fail.hpp"

namespace cero {

std::string_view unary_operator_to_string(UnaryOperator op) {
	switch (op) {
		using enum UnaryOperator;
		case PreInc:  return "prefix ++";
		case PreDec:  return "prefix --";
		case PostInc: return "postfix ++";
		case PostDec: return "postfix --";
		case Addr:	  return "&";
		case Deref:	  return "^";
		case Neg:	  return "-";
		case Not:	  return "~";
	}
	fail_unreachable();
}

std::string_view binary_operator_to_string(BinaryOperator op) {
	switch (op) {
		using enum BinaryOperator;
		case Add:			 return "+";
		case Sub:			 return "-";
		case Mul:			 return "*";
		case Div:			 return "/";
		case Rem:			 return "%";
		case Pow:			 return "**";
		case LogicAnd:		 return "&&";
		case LogicOr:		 return "||";
		case BitAnd:		 return "&";
		case BitOr:			 return "|";
		case Xor:			 return "~";
		case Shl:			 return "<<";
		case Shr:			 return ">>";
		case Eq:			 return "==";
		case NotEq:			 return "!=";
		case Less:			 return "<";
		case LessEq:		 return "<=";
		case Greater:		 return ">";
		case GreaterEq:		 return ">=";
		case Assign:		 return "=";
		case AddAssign:		 return "+=";
		case SubAssign:		 return "-=";
		case MulAssign:		 return "*=";
		case DivAssign:		 return "/=";
		case RemAssign:		 return "%=";
		case PowAssign:		 return "**=";
		case BitAndAssign:	 return "&=";
		case BitOrAssign:	 return "|=";
		case XorAssign:		 return "~=";
		case ShlAssign:		 return "<<=";
		case ShrAssign:		 return ">>=";
		case LogicAndAssign: return "&&=";
		case LogicOrAssign:	 return "||=";
	}
	fail_unreachable();
}

} // namespace cero
