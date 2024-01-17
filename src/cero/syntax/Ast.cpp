#include "Ast.hpp"

#include "cero/syntax/AstToString.hpp"

namespace cero {

uint32_t Ast::num_nodes() const {
	return static_cast<uint32_t>(nodes_.size());
}

bool Ast::has_errors() const {
	return has_errors_;
}

std::span<const AstNode> Ast::raw() const {
	return {nodes_};
}

std::string Ast::to_string(const SourceLock& source) const {
	AstToString ast_to_string(*this, source);
	return ast_to_string.make_string();
}

Ast::Ast(std::vector<AstNode>&& nodes) :
	nodes_(std::move(nodes)),
	has_errors_(false) {
}

} // namespace cero
