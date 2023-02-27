#pragma once

#include "cero/syntax/Definition.hpp"
#include "cero/syntax/Expression.hpp"

#include <vector>

namespace cero
{

class AstBuilder
{
	friend class SyntaxTree;

	std::vector<ExpressionNode> expression_nodes;
	std::vector<DefinitionNode> definition_nodes;
	ast::Root					root;

public:
	const ExpressionNode& get(Expression expression) const;
	const DefinitionNode& get(Definition definition) const;

	void	   add_to_root(Definition definition);
	Expression store(ExpressionNode node);
	Definition store(DefinitionNode node);
};

} // namespace cero
