#include "AstCursor.hpp"

#include "cero/util/Macros.hpp"

namespace cero {

AstCursor::AstCursor(const Ast& ast) :
	it_(ast.raw().begin()),
	num_children_to_visit_(it_->num_children()) {
}

void AstCursor::visit_all(AstVisitor& visitor) {
	const uint32_t old_num = std::exchange(num_children_to_visit_, it_->num_children());
	it_++->visit(visitor);

	while (num_children_to_visit_ > 0) {
		visit_all(visitor);
		--num_children_to_visit_;
	}

	num_children_to_visit_ = old_num;
}

void AstCursor::visit_child(AstVisitor& visitor) {
	CERO_ASSERT_DEBUG(num_children_to_visit_ > 0, "Attempted to visit child but current node has no children.");

	if (num_children_to_visit_ > 0) {
		visit_all(visitor);
		--num_children_to_visit_;
	}
}

void AstCursor::visit_children(uint32_t n, AstVisitor& visitor) {
	CERO_ASSERT_DEBUG(n <= num_children_to_visit_, "Attempted to visit more children than the current node has left to visit.");
	n = std::min(n, num_children_to_visit_);

	while (n > 0) {
		visit_all(visitor);
		--num_children_to_visit_;
		--n;
	}
}

uint32_t AstCursor::num_children_to_visit() const {
	return num_children_to_visit_;
}

} // namespace cero
