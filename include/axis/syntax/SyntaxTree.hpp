#pragma once

#include "cero/driver/Source.hpp"
#include "cero/syntax/AstBuilder.hpp"
#include "cero/syntax/AstVisitor.hpp"

namespace cero
{

class SyntaxTree
{
	std::vector<ExpressionNode> expression_nodes;
	std::vector<DefinitionNode> definition_nodes;
	ast::Root					root;

public:
	explicit SyntaxTree(AstBuilder builder);

	void visit(AstVisitor& visitor) const;
	void visit(AstVisitor& visitor, Expression expression) const;
	void visit(AstVisitor& visitor, Definition definition) const;

	bool operator==(const SyntaxTree& other) const;

	std::string to_string(const Source& source) const;
	void		log(const Source& source) const;
};

} // namespace cero
