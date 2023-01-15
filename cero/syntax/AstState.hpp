#pragma once

#include "syntax/Definition.hpp"
#include "syntax/Expression.hpp"

#include <span>
#include <vector>

namespace cero
{

class AstState
{
	std::vector<ExpressionNode> expression_nodes;
	std::vector<DefinitionNode> definition_nodes;
	std::vector<Definition>		root_definitions;

public:
	void	   add_to_root(Definition definition);
	Expression add(ExpressionNode node);
	Definition add(DefinitionNode node);

	std::span<const Definition> get_root_definitions() const;
	const ExpressionNode&		get(Expression expression) const;
	const DefinitionNode&		get(Definition definition) const;
};

} // namespace cero