#pragma once

#include "syntax/Expression.hpp"

struct Definition
{
	Index index;

	Definition(Index index) :
		index(index)
	{}
};

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
};

struct ReturnValue
{
	Expression		 type;
	std::string_view name;
};

struct Function
{
	std::string_view		 name;
	std::vector<Parameter>	 parameters;
	std::vector<ReturnValue> returns;
	std::vector<Expression>	 statements;
};

struct Struct
{};

struct Enum
{};

using DefinitionNode = std::variant<Function, Struct, Enum>;
