#pragma once

#include "syntax/Literal.hpp"

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

using Index = uint32_t;

constexpr inline Index NULL_INDEX = ~0u;

struct AstIndex
{
	Index index = NULL_INDEX;

	bool is_null() const
	{
		return index == NULL_INDEX;
	}
};

struct Expression : AstIndex
{};

struct UnaryExpression
{
	Expression expr;
};

struct BinaryExpression
{
	Expression left_expr;
	Expression right_expr;
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
	Expression count_expr;
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
	Expression				type_expr;
	std::vector<Expression> invalidation_layers;
};

struct LetBinding
{
	std::string_view name;
	Expression		 type;
	Expression		 initializer;
};

struct VarBinding
{
	std::string_view name;
	Expression		 type;
	Expression		 initializer;
};

struct BlockExpression
{
	std::vector<Expression> statements;
};

struct IfExpression
{
	Expression condition;
	Expression then_expression;
	Expression else_expression;
};

struct WhileLoop
{
	Expression condition;
	Expression statement;
};

struct ForLoop
{
	Expression _;
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

struct BreakExpression : UnaryExpression {};
struct ContinueExpression : UnaryExpression {};
struct ReturnExpression : UnaryExpression {};
struct ThrowExpression : UnaryExpression {};
struct TryExpression : UnaryExpression {};
struct PreIncrement : UnaryExpression {};
struct PreDecrement : UnaryExpression {};
struct PostIncrement : UnaryExpression {};
struct PostDecrement : UnaryExpression {};
struct LogicalNot : UnaryExpression {};
struct BitwiseNot : UnaryExpression {};
struct Negation : UnaryExpression {};
struct Dereference : UnaryExpression {};
struct AddressOf : UnaryExpression {};
struct Addition : BinaryExpression {};
struct Subtraction : BinaryExpression {};
struct Multiplication : BinaryExpression {};
struct Division : BinaryExpression {};
struct Remainder : BinaryExpression {};
struct Exponentiation : BinaryExpression {};
struct BitwiseAnd : BinaryExpression {};
struct BitwiseOr : BinaryExpression {};
struct Xor : BinaryExpression {};
struct LeftShift : BinaryExpression {};
struct RightShift : BinaryExpression {};
struct LogicalAnd : BinaryExpression {};
struct LogicalOr : BinaryExpression {};
struct Equality : BinaryExpression {};
struct Inequality : BinaryExpression {};
struct Less : BinaryExpression {};
struct Greater : BinaryExpression {};
struct LessEqual : BinaryExpression {};
struct GreaterEqual : BinaryExpression {};
struct Assignment : BinaryExpression {};
struct AdditionAssignment : BinaryExpression {};
struct SubtractionAssignment : BinaryExpression {};
struct MultiplicationAssignment : BinaryExpression {};
struct DivisionAssignment : BinaryExpression {};
struct RemainderAssignment : BinaryExpression {};
struct ExponentiationAssignment : BinaryExpression {};
struct BitwiseAndAssignment : BinaryExpression {};
struct BitwiseOrAssignment : BinaryExpression {};
struct XorAssignment : BinaryExpression {};
struct LeftShiftAssignment : BinaryExpression {};
struct RightShiftAssignment : BinaryExpression {};

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