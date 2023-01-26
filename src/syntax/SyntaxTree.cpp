#include "cero/syntax/SyntaxTree.hpp"

#include "syntax/AstComparator.hpp"
#include "syntax/AstString.hpp"

namespace cero
{

std::span<const Definition> SyntaxTree::get_root() const
{
	return root_definitions;
}

const ExpressionNode& SyntaxTree::get(Expression expression) const
{
	return expression_nodes.at(expression.index);
}

const DefinitionNode& SyntaxTree::get(Definition definition) const
{
	return definition_nodes.at(definition.index);
}

bool SyntaxTree::operator==(const SyntaxTree& other) const
{
	AstComparator comparator(*this, other);
	return comparator.compare();
}

std::string SyntaxTree::to_string(const Source& source) const
{
	AstString ast_string(*this, source);
	return ast_string.build();
}

void SyntaxTree::log(const Source& source) const
{
	std::clog << to_string(source);
}

void SyntaxTree::add_to_root(Definition definition)
{
	root_definitions.emplace_back(definition);
}

Expression SyntaxTree::add(ExpressionNode node)
{
	expression_nodes.emplace_back(std::move(node));
	auto index = static_cast<AstIndex>(expression_nodes.size() - 1);
	return Expression(index);
}

Definition SyntaxTree::add(DefinitionNode node)
{
	definition_nodes.emplace_back(std::move(node));
	auto index = static_cast<AstIndex>(definition_nodes.size() - 1);
	return Definition(index);
}

} // namespace cero