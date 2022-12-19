#pragma once

#include "syntax/Literal.hpp"
#include "util/StringInteger.hpp"

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

using Index = uint32_t;

struct Expression
{
	Index index;

	Expression(Index index) :
		index(index)
	{}
};

class OptionalExpression
{
	static constexpr Index NULL_INDEX = ~0u;

	Expression expression;

public:
	OptionalExpression(Expression expression = NULL_INDEX) :
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

	Specifier				specifier = {};
	std::vector<Expression> arguments;
};

struct ArrayTypeExpression
{
	OptionalExpression count;
	Expression		   element_type;
};

struct PointerTypeExpression
{
	Variability variability;
	Expression	type;
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

struct BlockExpression
{
	std::vector<Expression> statements;
};

struct GroupExpression
{
	Expression expression;
};

struct IfExpression
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

struct BreakExpression
{
	OptionalExpression label;
};

struct ContinueExpression
{
	OptionalExpression label;
};

struct ReturnExpression
{
	OptionalExpression expression;
};

struct ThrowExpression
{
	OptionalExpression expression;
};

struct MemberAccess
{
	Expression		 target;
	std::string_view member;
};

struct CallExpression
{
	Expression				callee;
	std::vector<Expression> arguments;
};

struct IndexExpression
{
	Expression				target;
	std::vector<Expression> arguments;
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

using ExpressionNode = std::variant<Identifier,
									GenericIdentifier,
									Variability,
									ArrayTypeExpression,
									PointerTypeExpression,
									NumericLiteral,
									StringLiteral,
									Binding,
									BlockExpression,
									GroupExpression,
									IfExpression,
									WhileLoop,
									ForLoop,
									BreakExpression,
									ContinueExpression,
									ReturnExpression,
									ThrowExpression,
									MemberAccess,
									CallExpression,
									IndexExpression,
									UnaryExpression,
									BinaryExpression>;
