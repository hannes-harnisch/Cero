#pragma once

#include "cero/syntax/Ast.hpp"

#include <span>

namespace cero {

/// Utility for traversing an AST. Its methods are reentrant, meaning you can call a visit method recursively while a visit
/// method on the same cursor has not yet completed.
class AstCursor {
public:
	/// Creates a cursor positioned at the root of the given AST.
	explicit AstCursor(const Ast& ast);

	/// Traverse the entire AST using the given visitor.
	void visit_all(AstVisitor& visitor);

	/// Visit one child of the current node and all children of that child node.
	void visit_child(AstVisitor& visitor);

	/// Visit the given number of children of the current node and all children of those child nodes.
	void visit_children(uint32_t n, AstVisitor& visitor);

	/// Gets the number of children of the current node that have not yet been visited.
	uint32_t num_children_to_visit() const;

private:
	std::span<const AstNode>::iterator it_;
	uint32_t num_children_to_visit_;
};

} // namespace cero
