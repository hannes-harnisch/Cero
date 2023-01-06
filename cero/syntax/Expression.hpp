#pragma once

#include "syntax/Literal.hpp"

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

using Index = uint32_t;

struct Expression
{
	Index index;

	explicit Expression(Index index) :
		index(index)
	{}

	bool operator==(const Expression&) const = default;
};

class OptionalExpression
{
	static constexpr Index NULL_INDEX = ~0u;

	Expression expression;

public:
	OptionalExpression() :
		expression(NULL_INDEX)
	{}

	explicit OptionalExpression(Expression expression) :
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

	bool operator==(const OptionalExpression&) const = default;
};

struct Identifier
{
	std::string_view name;

	bool operator==(const Identifier&) const = default;
};

struct GenericIdentifier
{
	std::string_view		name;
	std::vector<Expression> arguments;

	bool operator==(const GenericIdentifier&) const = default;
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

	bool operator==(const Variability&) const = default;
};

struct ArrayTypeExpression
{
	OptionalExpression count;
	Expression		   element_type;

	bool operator==(const ArrayTypeExpression&) const = default;
};

struct PointerTypeExpression
{
	Variability variability;
	Expression	type;

	bool operator==(const PointerTypeExpression&) const = default;
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

	bool operator==(const Binding&) const = default;
};

struct BlockExpression
{
	std::vector<Expression> statements;

	bool operator==(const BlockExpression&) const = default;
};

struct IfExpression
{
	Expression		   condition;
	Expression		   then_expression;
	OptionalExpression else_expression;

	bool operator==(const IfExpression&) const = default;
};

struct WhileLoop
{
	Expression condition;
	Expression statement;

	bool operator==(const WhileLoop&) const = default;
};

struct ForLoop
{
	Expression binding;
	Expression range_expression;
	Expression statement;

	bool operator==(const ForLoop&) const = default;
};

struct BreakExpression
{
	OptionalExpression label;

	bool operator==(const BreakExpression&) const = default;
};

struct ContinueExpression
{
	OptionalExpression label;

	bool operator==(const ContinueExpression&) const = default;
};

struct ReturnExpression
{
	OptionalExpression expression;

	bool operator==(const ReturnExpression&) const = default;
};

struct ThrowExpression
{
	OptionalExpression expression;

	bool operator==(const ThrowExpression&) const = default;
};

struct MemberAccess
{
	Expression		 target;
	std::string_view member;

	bool operator==(const MemberAccess&) const = default;
};

struct CallExpression
{
	OptionalExpression		callee;
	std::vector<Expression> arguments;

	bool operator==(const CallExpression&) const = default;
};

struct IndexExpression
{
	Expression				target;
	std::vector<Expression> arguments;

	bool operator==(const IndexExpression&) const = default;
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

	bool operator==(const UnaryExpression&) const = default;
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

	bool operator==(const BinaryExpression&) const = default;
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
