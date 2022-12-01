#pragma once

#include "syntax/Definition.hpp"
#include "syntax/Expression.hpp"

class SyntaxTree
{
	std::vector<ExpressionNode> expression_nodes;
	std::vector<DefinitionNode> definition_nodes;

public:
	Expression add(ExpressionNode expr);
	Definition add(DefinitionNode def);
	void	   print() const;
};
