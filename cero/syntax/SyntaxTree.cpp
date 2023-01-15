#include "SyntaxTree.hpp"

#include "syntax/AstComparator.hpp"
#include "syntax/AstString.hpp"

namespace cero
{

SyntaxTree::SyntaxTree(AstState state) :
	state(std::move(state))
{}

std::span<const Definition> SyntaxTree::get_root() const
{
	return state.get_root_definitions();
}

const ExpressionNode& SyntaxTree::get(Expression expression) const
{
	return state.get(expression);
}

bool SyntaxTree::operator==(const SyntaxTree& other) const
{
	AstComparator comparator(*this, other);
	return comparator.compare();
}

const DefinitionNode& SyntaxTree::get(Definition definition) const
{
	return state.get(definition);
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

} // namespace cero