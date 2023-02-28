#include "cero/syntax/AstBuilder.hpp"

namespace cero
{

const Expression& AstBuilder::get(ExpressionId expression) const
{
	return expression_nodes.at(expression.id);
}

const Definition& AstBuilder::get(DefinitionId definition) const
{
	return definition_nodes.at(definition.id);
}

void AstBuilder::add_to_root(DefinitionId definition)
{
	root.root_definitions.emplace_back(definition);
}

ExpressionId AstBuilder::store(Expression node)
{
	expression_nodes.emplace_back(std::move(node));
	auto id = static_cast<AstId>(expression_nodes.size() - 1);
	return ExpressionId(id);
}

DefinitionId AstBuilder::store(Definition node)
{
	definition_nodes.emplace_back(std::move(node));
	auto id = static_cast<AstId>(definition_nodes.size() - 1);
	return DefinitionId(id);
}

} // namespace cero
