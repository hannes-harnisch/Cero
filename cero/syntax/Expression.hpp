#pragma once

#include "syntax/Literal.hpp"

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace cero
{

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

namespace ast
{
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

	struct ArrayType
	{
		OptionalExpression bound;
		Expression		   element_type;

		bool operator==(const ArrayType&) const = default;
	};

	struct PointerType
	{
		Variability variability;
		Expression	type;

		bool operator==(const PointerType&) const = default;
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

	struct Block
	{
		std::vector<Expression> statements;

		bool operator==(const Block&) const = default;
	};

	struct If
	{
		Expression		   condition;
		Expression		   then_expression;
		OptionalExpression else_expression;

		bool operator==(const If&) const = default;
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

	struct Break
	{
		OptionalExpression label;

		bool operator==(const Break&) const = default;
	};

	struct Continue
	{
		OptionalExpression label;

		bool operator==(const Continue&) const = default;
	};

	struct Return
	{
		OptionalExpression expression;

		bool operator==(const Return&) const = default;
	};

	struct Throw
	{
		OptionalExpression expression;

		bool operator==(const Throw&) const = default;
	};

	struct MemberAccess
	{
		Expression		 target;
		std::string_view member;

		bool operator==(const MemberAccess&) const = default;
	};

	struct Call
	{
		OptionalExpression		callee;
		std::vector<Expression> arguments;

		bool operator==(const Call&) const = default;
	};

	struct Index
	{
		Expression				target;
		std::vector<Expression> arguments;

		bool operator==(const Index&) const = default;
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
} // namespace ast

using ExpressionNode = std::variant<ast::Identifier,
									ast::GenericIdentifier,
									ast::Variability,
									ast::ArrayType,
									ast::PointerType,
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
