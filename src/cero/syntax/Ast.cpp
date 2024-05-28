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

std::string Ast::to_string(const SourceGuard& source) const {
	return AstToString(*this, source).make_string();
}

Ast::Ast(const TokenStream& token_stream) :
	has_errors_(false) {
	nodes_.reserve(token_stream.num_tokens());
}

void Ast::insert_parent(Ast::NodeIndex first_descendant_index, AstNode&& node) {
	nodes_.insert(nodes_.begin() + static_cast<ptrdiff_t>(first_descendant_index), std::move(node));
}

Ast::NodeIndex Ast::next_index() const {
	return static_cast<NodeIndex>(nodes_.size());
}

} // namespace cero
