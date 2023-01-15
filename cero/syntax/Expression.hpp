#pragma once

#include "syntax/Literal.hpp"

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace cero
{

using AstIndex = uint32_t;

struct Expression
{
	AstIndex index;

	explicit Expression(AstIndex index) :
		index(index)
	{}
};

class OptionalExpression
{
	static constexpr AstIndex NULL_INDEX = ~0u;

	Expression expression;

public:
	OptionalExpression() :
		expression(NULL_INDEX)
	{}

	OptionalExpression(Expression expression) :
		expression(expression)
	{}

	bool is_null() const
	{
		return expression.index == NULL_INDEX;
	}

	Expression get() const
	{
		return expression;
	}
};

namespace ast
{
	struct Identifier
	{
		std::string_view name;
	};

	struct GenericIdentifier
	{
		std::string_view		name;
		std::vector<Expression> arguments;
	};

	struct Variability
	{
		enum class Specifier : uint8_t
		{
			In,
			Var,
			VarBounded,
			VarUnbounded
		};

		Specifier				specifier = Specifier::In;
		std::vector<Expression> arguments;
	};

	struct ArrayType
	{
		OptionalExpression bound;
		Expression		   element_type;
	};

	struct PointerType
	{
		Variability variability;
		Expression	type;
	};

	enum class ParameterSpecifier : uint8_t
	{
		In,
		Let,
		Var
	};

	struct ReturnValue
	{
		Expression		 type;
		std::string_view name;
	};

	struct FunctionType
	{
		struct Parameter
		{
			ParameterSpecifier specifier = {};
			std::string_view   name;
			Expression		   type;
		};

		std::vector<Parameter>	 parameters;
		std::vector<ReturnValue> returns;
	};

	struct Binding
	{
		enum class Specifier
		{
			Let,
			Var,
			Const,
			Static,
			StaticVar
		};

		Specifier		   specifier = {};
		std::string_view   name;
		OptionalExpression type;
		OptionalExpression initializer;
	};

	struct Block
	{
		std::vector<Expression> statements;
	};

	struct If
	{
		Expression		   condition;
		Expression		   then_expression;
		OptionalExpression else_expression;
	};

	struct WhileLoop
	{
		Expression condition;
		Expression statement;
	};

	struct ForLoop
	{
		Expression binding;
		Expression range_expression;
		Expression statement;
	};

	struct Break
	{
		OptionalExpression label;
	};

	struct Continue
	{
		OptionalExpression label;
	};

	struct Return
	{
		OptionalExpression expression;
	};

	struct Throw
	{
		OptionalExpression expression;
	};

	struct MemberAccess
	{
		Expression		 target;
		std::string_view member;
	};

	struct Call
	{
		OptionalExpression		callee;
		std::vector<Expression> arguments;
	};

	struct Index
	{
		Expression				target;
		std::vector<Expression> arguments;
	};

	struct ArrayLiteral
	{
		std::vector<Expression> elements;
	};

	enum class UnaryOperator
	{
		TryOperator,
		PreIncrement,
		PreDecrement,
		PostIncrement,
		PostDecrement,
		AddressOf,
		Dereference,
		Negation,
		LogicalNot,
		BitwiseNot
	};

	struct UnaryExpression
	{
		UnaryOperator op;
		Expression	  operand;
	};

	enum class BinaryOperator
	{
		Add,
		Subtract,
		Multiply,
		Divide,
		Remainder,
		Power,
		LogicalAnd,
		LogicalOr,
		BitAnd,
		BitOr,
		Xor,
		LeftShift,
		RightShift,
		Equality,
		Inequality,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,
		Assign,
		AddAssign,
		SubtractAssign,
		MultiplyAssign,
		DivideAssign,
		RemainderAssign,
		PowerAssign,
		BitAndAssign,
		BitOrAssign,
		XorAssign,
		LeftShiftAssign,
		RightShiftAssign
	};

	struct BinaryExpression
	{
		BinaryOperator op;
		Expression	   left;
		Expression	   right;
	};
} // namespace ast

using ExpressionNode = std::variant<ast::Identifier,
									ast::GenericIdentifier,
									ast::Variability,
									ast::ArrayType,
									ast::PointerType,
									ast::FunctionType,
									ast::NumericLiteral,
									ast::StringLiteral,
									ast::Binding,
									ast::Block,
									ast::If,
									ast::WhileLoop,
									ast::ForLoop,
									ast::Break,
									ast::Continue,
									ast::Return,
									ast::Throw,
									ast::MemberAccess,
									ast::Call,
									ast::Index,
									ast::UnaryExpression,
									ast::BinaryExpression>;

} // namespace cero
