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
	struct Root
	{
		std::vector<Definition> root_definitions;
	};

	enum class AccessSpecifier : uint8_t
	{
		None,
		Private,
		Public
	};

	struct Function
	{
		struct Parameter
		{
			ParameterSpecifier specifier = {};
			std::string_view   name;
			Expression		   type;
			OptionalExpression default_argument;
		};

		AccessSpecifier				access;
		std::string_view			name;
		std::vector<Parameter>		parameters;
		std::vector<FunctionOutput> outputs;
		std::vector<Expression>		statements;
	};

	struct Struct
	{
		AccessSpecifier	 access;
		std::string_view name;
	};

	struct Enum
	{
		AccessSpecifier	 access;
		std::string_view name;
	};
} // namespace ast

using DefinitionNode = std::variant<ast::Function, ast::Struct, ast::Enum>;

} // namespace cero
