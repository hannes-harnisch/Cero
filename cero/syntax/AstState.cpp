#include "AstState.hpp"

namespace cero
{

void AstState::add_to_root(Definition definition)
{
	root_definitions.emplace_back(definition);
}

Expression AstState::add(ExpressionNode node)
{
	expression_nodes.emplace_back(std::move(node));
	auto index = static_cast<AstIndex>(expression_nodes.size() - 1);
	return Expression(index);
}

Definition AstState::add(DefinitionNode node)
{
	definition_nodes.emplace_back(std::move(node));
	auto index = static_cast<AstIndex>(definition_nodes.size() - 1);
	return Definition(index);
}

std::span<const Definition> AstState::get_root_definitions() const
{
	return root_definitions;
}

const ExpressionNode& AstState::get(Expression expression) const
{
	return expression_nodes.at(expression.index);
}

const DefinitionNode& AstState::get(Definition definition) const
{
	return definition_nodes.at(definition.index);
}

} // namespace cero