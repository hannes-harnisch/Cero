#pragma once

#include "cero/syntax/Expression.hpp"

namespace cero
{

struct DefinitionId
{
	AstId id;

	explicit DefinitionId(AstId id) :
		id(id)
	{}
};

namespace ast
{
	struct Root
	{
		std::vector<DefinitionId> root_definitions;
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
			ParameterSpecifier	 specifier = {};
			std::string_view	 name;
			ExpressionId		 type;
			OptionalExpressionId default_argument;
		};

		AccessSpecifier				access;
		std::string_view			name;
		std::vector<Parameter>		parameters;
		std::vector<FunctionOutput> outputs;
		std::vector<ExpressionId>	statements;
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

using Definition = std::variant<ast::Function, ast::Struct, ast::Enum>;

} // namespace cero
