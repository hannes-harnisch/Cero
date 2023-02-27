#include "cero/syntax/AstBuilder.hpp"

namespace cero
{

const ExpressionNode& AstBuilder::get(Expression expression) const
{
	return expression_nodes.at(expression.index);
}

const DefinitionNode& AstBuilder::get(Definition definition) const
{
	return definition_nodes.at(definition.index);
}

void AstBuilder::add_to_root(Definition definition)
{
	root.root_definitions.emplace_back(definition);
}

Expression AstBuilder::store(ExpressionNode node)
{
	expression_nodes.emplace_back(std::move(node));
	auto index = static_cast<AstIndex>(expression_nodes.size() - 1);
	return Expression(index);
}

Definition AstBuilder::store(DefinitionNode node)
{
	definition_nodes.emplace_back(std::move(node));
	auto index = static_cast<AstIndex>(definition_nodes.size() - 1);
	return Definition(index);
}

} // namespace cero
