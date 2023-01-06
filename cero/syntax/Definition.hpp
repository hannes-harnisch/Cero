#pragma once

#include "syntax/Expression.hpp"

struct Definition
{
	Index index;

	explicit Definition(Index index) :
		index(index)
	{}

	bool operator==(const Definition&) const = default;
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
	bool operator==(const Struct&) const = default;
};

struct Enum
{
	bool operator==(const Enum&) const = default;
};

using DefinitionNode = std::variant<Function, Struct, Enum>;
