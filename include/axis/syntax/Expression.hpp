#pragma once

#include "cero/syntax/Literal.hpp"

#include <cstdint>
#include <string_view>
#include <variant>
#include <vector>

namespace cero
{

using AstId = uint32_t;

struct ExpressionId
{
	AstId id;

	explicit ExpressionId(AstId id) :
		id(id)
	{}
};

class OptionalExpressionId
{
	static constexpr AstId NULL_ID = ~0u;

	ExpressionId expression;

public:
	OptionalExpressionId() :
		expression(NULL_ID)
	{}

	OptionalExpressionId(ExpressionId expression) :
		expression(expression)
	{}

	bool is_null() const
	{
		return expression.id == NULL_ID;
	}

	ExpressionId get() const
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
		std::string_view		  name;
		std::vector<ExpressionId> arguments;
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

		Specifier				  specifier = Specifier::In;
		std::vector<ExpressionId> arguments;
	};

	struct ArrayType
	{
		OptionalExpressionId bound;
		ExpressionId		 element_type;
	};

	struct PointerType
	{
		Variability	 variability;
		ExpressionId type;
	};

	enum class ParameterSpecifier : uint8_t
	{
		In,
		Let,
		Var
	};

	struct FunctionOutput
	{
		ExpressionId	 type;
		std::string_view name;
	};

	struct FunctionType
	{
		struct Parameter
		{
			ParameterSpecifier specifier = {};
			std::string_view   name;
			ExpressionId	   type;
		};

		std::vector<Parameter>		parameters;
		std::vector<FunctionOutput> outputs;
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

		Specifier			 specifier = {};
		std::string_view	 name;
		OptionalExpressionId type;
		OptionalExpressionId initializer;
	};

	struct Block
	{
		std::vector<ExpressionId> statements;
	};

	struct If
	{
		ExpressionId		 condition;
		ExpressionId		 then_expression;
		OptionalExpressionId else_expression;
	};

	struct WhileLoop
	{
		ExpressionId condition;
		ExpressionId statement;
	};

	struct ForLoop
	{
		ExpressionId binding;
		ExpressionId range_expression;
		ExpressionId statement;
	};

	struct Break
	{
		OptionalExpressionId label;
	};

	struct Continue
	{
		OptionalExpressionId label;
	};

	struct Return
	{
		OptionalExpressionId expression;
	};

	struct Throw
	{
		OptionalExpressionId expression;
	};

	struct MemberAccess
	{
		ExpressionId	 target;
		std::string_view member;
	};

	struct Group
	{
		std::vector<ExpressionId> arguments;
	};

	struct Call
	{
		OptionalExpressionId	  callee;
		std::vector<ExpressionId> arguments;
	};

	struct Index
	{
		ExpressionId			  target;
		std::vector<ExpressionId> arguments;
	};

	struct ArrayLiteral
	{
		std::vector<ExpressionId> elements;
	};

	enum class UnaryOperator
	{
		PreIncrement,
		PreDecrement,
		PostIncrement,
		PostDecrement,
		AddressOf,
		Dereference,
		Negate,
		LogicalNot,
		BitwiseNot
	};

	struct UnaryExpression
	{
		UnaryOperator op;
		ExpressionId  operand;
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
		Equal,
		NotEqual,
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
		AndAssign,
		OrAssign,
		XorAssign,
		LeftShiftAssign,
		RightShiftAssign
	};

	struct BinaryExpression
	{
		BinaryOperator op;
		ExpressionId   left;
		ExpressionId   right;
	};
} // namespace ast

using Expression = std::variant<ast::Identifier,
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
