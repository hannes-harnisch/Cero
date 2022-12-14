#include "SyntaxTree.hpp"

#include "syntax/AstString.hpp"

void SyntaxTree::add_to_root(Definition definition)
{
	root_definitions.emplace_back(definition);
}

Expression SyntaxTree::add(ExpressionNode node)
{
	expression_nodes.emplace_back(std::move(node));
	return {static_cast<Index>(expression_nodes.size() - 1)};
}

Definition SyntaxTree::add(DefinitionNode node)
{
	definition_nodes.emplace_back(std::move(node));
	return {static_cast<Index>(definition_nodes.size() - 1)};
}

std::span<const Definition> SyntaxTree::get_root_definitions() const
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

std::string SyntaxTree::to_string(const Source& source) const
{
	AstString ast_string(*this, source);
	return ast_string.build();
}

void SyntaxTree::log(const Source& source) const
{
	std::clog << to_string(source);
}
