#pragma once

#include "syntax/Expression.hpp"

namespace cero
{

struct Definition
{
	Index index;

	explicit Definition(Index index) :
		index(index)
	{}

	bool operator==(const Definition&) const = default;
};

namespace ast
{
	enum class ParameterKind : uint8_t
	{
		In,
		Let,
		Var
	};

	struct Parameter
	{
		ParameterKind	   kind = {};
		std::string_view   name;
		Expression		   type;
		OptionalExpression default_argument;

		bool operator==(const Parameter&) const = default;
	};

	struct ReturnValue
	{
		Expression		 type;
		std::string_view name;

		bool operator==(const ReturnValue&) const = default;
	};

	struct Function
	{
		std::string_view		 name;
		std::vector<Parameter>	 parameters;
		std::vector<ReturnValue> returns;
		std::vector<Expression>	 statements;

		bool operator==(const Function&) const = default;
	};

	struct Struct
	{
		std::string_view name;

		bool operator==(const Struct&) const = default;
	};

	struct Enum
	{
		std::string_view name;

		bool operator==(const Enum&) const = default;
	};
} // namespace ast

using DefinitionNode = std::variant<ast::Function, ast::Struct, ast::Enum>;

} // namespace cero
