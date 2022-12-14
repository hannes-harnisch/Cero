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

	Expression(Index index) :
		index(index)
	{}
};

struct OptionalExpression : Expression
{
	static constexpr Index NULL_INDEX = ~0u;

	OptionalExpression(Expression expr = NULL_INDEX) :
		Expression(expr)
	{}

	bool is_null() const
	{
		return index == NULL_INDEX;
	}
};

struct BinaryExpression
{
	Expression left;
	Expression right;
};

struct Identifier
{
	std::string_view name;
};

struct GenericIdentifier
{
	std::string_view		name;
	std::vector<Expression> generic_args;
};

struct ArrayTypeExpression
{
	Expression count_expression;
	Expression element_type;
};

enum class VarSpecifier : uint8_t
{
	None,
	VarDefault,
	VarFinite,
	VarInfinite
};

struct PointerTypeExpression
{
	VarSpecifier			var_specifier;
	Expression				type;
	std::vector<Expression> invalidation_layers;
};

struct LetBinding
{
	std::string_view   name;
	OptionalExpression type;
	OptionalExpression initializer;
};

struct VarBinding
{
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

struct Access
{
	Expression		 target;
	std::string_view member;
};

struct Call
{
	Expression				callee;
	std::vector<Expression> arguments;
};

struct Indexing
{
	Expression				target;
	std::vector<Expression> arguments;
};

// clang-format off

struct BreakExpression			: OptionalExpression {};
struct ContinueExpression		: OptionalExpression {};
struct ReturnExpression			: OptionalExpression {};
struct ThrowExpression			: OptionalExpression {};
struct TryExpression			: Expression {};
struct PreIncrement				: Expression {};
struct PreDecrement				: Expression {};
struct PostIncrement			: Expression {};
struct PostDecrement			: Expression {};
struct LogicalNot				: Expression {};
struct BitwiseNot				: Expression {};
struct Negation					: Expression {};
struct Dereference				: Expression {};
struct AddressOf				: Expression {};
struct Addition					: BinaryExpression {};
struct Subtraction				: BinaryExpression {};
struct Multiplication			: BinaryExpression {};
struct Division					: BinaryExpression {};
struct Remainder				: BinaryExpression {};
struct Exponentiation			: BinaryExpression {};
struct BitwiseAnd				: BinaryExpression {};
struct BitwiseOr				: BinaryExpression {};
struct Xor						: BinaryExpression {};
struct LeftShift				: BinaryExpression {};
struct RightShift				: BinaryExpression {};
struct LogicalAnd				: BinaryExpression {};
struct LogicalOr				: BinaryExpression {};
struct Equality					: BinaryExpression {};
struct Inequality				: BinaryExpression {};
struct Less						: BinaryExpression {};
struct Greater					: BinaryExpression {};
struct LessEqual				: BinaryExpression {};
struct GreaterEqual				: BinaryExpression {};
struct Assignment				: BinaryExpression {};
struct AdditionAssignment		: BinaryExpression {};
struct SubtractionAssignment	: BinaryExpression {};
struct MultiplicationAssignment	: BinaryExpression {};
struct DivisionAssignment		: BinaryExpression {};
struct RemainderAssignment		: BinaryExpression {};
struct ExponentiationAssignment	: BinaryExpression {};
struct BitwiseAndAssignment		: BinaryExpression {};
struct BitwiseOrAssignment		: BinaryExpression {};
struct XorAssignment			: BinaryExpression {};
struct LeftShiftAssignment		: BinaryExpression {};
struct RightShiftAssignment		: BinaryExpression {};

// clang-format on

using ExpressionNode = std::variant<Identifier,
									GenericIdentifier,
									ArrayTypeExpression,
									PointerTypeExpression,
									NumericLiteral,
									StringLiteral,
									LetBinding,
									VarBinding,
									BlockExpression,
									GroupExpression,
									IfExpression,
									WhileLoop,
									ForLoop,
									Access,
									Call,
									Indexing,
									BreakExpression,
									ContinueExpression,
									ReturnExpression,
									ThrowExpression,
									TryExpression,
									PreIncrement,
									PreDecrement,
									PostIncrement,
									PostDecrement,
									AddressOf,
									Dereference,
									Negation,
									LogicalNot,
									BitwiseNot,
									Addition,
									Subtraction,
									Multiplication,
									Division,
									Remainder,
									Exponentiation,
									LogicalAnd,
									LogicalOr,
									BitwiseAnd,
									BitwiseOr,
									Xor,
									LeftShift,
									RightShift,
									Equality,
									Inequality,
									Less,
									Greater,
									LessEqual,
									GreaterEqual,
									Assignment,
									AdditionAssignment,
									SubtractionAssignment,
									MultiplicationAssignment,
									DivisionAssignment,
									RemainderAssignment,
									ExponentiationAssignment,
									BitwiseAndAssignment,
									BitwiseOrAssignment,
									XorAssignment,
									LeftShiftAssignment,
									RightShiftAssignment>;
