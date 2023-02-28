#pragma once

#include "cero/syntax/Definition.hpp"
#include "cero/syntax/Expression.hpp"

#include <vector>

namespace cero
{

class AstBuilder
{
	friend class SyntaxTree;

	std::vector<Expression> expression_nodes;
	std::vector<Definition> definition_nodes;
	ast::Root				root;

public:
	const Expression& get(ExpressionId expression) const;
	const Definition& get(DefinitionId definition) const;

	void		 add_to_root(DefinitionId definition);
	ExpressionId store(Expression node);
	DefinitionId store(Definition node);
};

} // namespace cero
