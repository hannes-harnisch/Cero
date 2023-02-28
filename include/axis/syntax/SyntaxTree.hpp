#pragma once

#include "cero/driver/Source.hpp"
#include "cero/syntax/AstBuilder.hpp"
#include "cero/syntax/AstVisitor.hpp"

namespace cero
{

class SyntaxTree
{
	std::vector<Expression> expression_nodes;
	std::vector<Definition> definition_nodes;
	ast::Root				root;

public:
	explicit SyntaxTree(AstBuilder builder);

	void visit(AstVisitor& visitor) const;
	void visit(AstVisitor& visitor, ExpressionId expression) const;
	void visit(AstVisitor& visitor, DefinitionId definition) const;

	bool operator==(const SyntaxTree& other) const;

	std::string to_string(const Source& source) const;
	void		log(const Source& source) const;
};

} // namespace cero
