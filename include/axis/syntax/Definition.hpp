#pragma once

#include "cero/syntax/Expression.hpp"

namespace cero
{

struct Definition
{
	AstIndex index;

	explicit Definition(AstIndex index) :
		index(index)
	{}
};

namespace ast
{
	struct Function
	{
		struct Parameter
		{
			ParameterSpecifier specifier = {};
			std::string_view   name;
			Expression		   type;
			OptionalExpression default_argument;
		};

		std::string_view		 name;
		std::vector<Parameter>	 parameters;
		std::vector<ReturnValue> returns;
		std::vector<Expression>	 statements;
	};

	struct Struct
	{
		std::string_view name;
	};

	struct Enum
	{
		std::string_view name;
	};
} // namespace ast

using DefinitionNode = std::variant<ast::Function, ast::Struct, ast::Enum>;

} // namespace cero
