#pragma once

#include "cero/driver/Source.hpp"
#include "cero/syntax/Definition.hpp"
#include "cero/syntax/Expression.hpp"

namespace cero
{

class SyntaxTree
{
	std::vector<ExpressionNode> expression_nodes;
	std::vector<DefinitionNode> definition_nodes;
	std::vector<Definition>		root_definitions;

public:
	std::span<const Definition> get_root() const;
	const ExpressionNode&		get(Expression expression) const;
	const DefinitionNode&		get(Definition definition) const;

	bool operator==(const SyntaxTree& other) const;

	std::string to_string(const Source& source) const;
	void		log(const Source& source) const;

	void	   add_to_root(Definition definition);
	Expression add(ExpressionNode node);
	Definition add(DefinitionNode node);
};

} // namespace cero