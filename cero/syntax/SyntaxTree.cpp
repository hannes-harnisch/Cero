#include "SyntaxTree.hpp"

Expression SyntaxTree::add(ExpressionNode expr)
{
	expression_nodes.emplace_back(std::move(expr));
	return {static_cast<Index>(expression_nodes.size())};
}

Definition SyntaxTree::add(DefinitionNode def)
{
	definition_nodes.emplace_back(std::move(def));
	return {static_cast<Index>(definition_nodes.size())};
}

void SyntaxTree::print() const
{}
