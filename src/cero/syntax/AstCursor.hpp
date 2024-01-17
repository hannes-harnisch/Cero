#pragma once

#include "cero/syntax/Ast.hpp"

#include <span>

namespace cero {

class AstCursor {
public:
	explicit AstCursor(const Ast& ast);

	void visit_all(AstVisitor& visitor);

	void visit_child(AstVisitor& visitor);

	void visit_children(uint32_t n, AstVisitor& visitor);

	uint32_t num_children_to_visit() const;

private:
	std::span<const AstNode>::iterator it_;
	uint32_t num_children_to_visit_;
};

} // namespace cero
