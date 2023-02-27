#include "cero/syntax/SyntaxTree.hpp"

#include "syntax/AstComparator.hpp"
#include "syntax/AstString.hpp"

namespace cero
{

SyntaxTree::SyntaxTree(AstBuilder builder) :
	expression_nodes(std::move(builder.expression_nodes)),
	definition_nodes(std::move(builder.definition_nodes)),
	root(std::move(builder.root))
{}

void SyntaxTree::visit(AstVisitor& visitor) const
{
	visitor.visit(root);
}

void SyntaxTree::visit(AstVisitor& visitor, Expression expression) const
{
	auto& ast_node = expression_nodes.at(expression.index);
	std::visit([&](auto& node) { visitor.visit(node); }, ast_node);
}

void SyntaxTree::visit(AstVisitor& visitor, Definition definition) const
{
	auto& ast_node = definition_nodes.at(definition.index);
	std::visit([&](auto& node) { visitor.visit(node); }, ast_node);
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
	std::cout << to_string(source);
}

} // namespace cero
