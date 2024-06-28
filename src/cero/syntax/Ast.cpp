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

Ast::Ast(const TokenStream& token_stream) {
	nodes_.reserve(token_stream.num_tokens());
}

Ast::NodeIndex Ast::store(AstNode&& node) {
	auto index = static_cast<NodeIndex>(nodes_.size());
	nodes_.emplace_back(std::move(node));
	return index;
}

Ast::NodeIndex Ast::store_parent_of(NodeIndex first_child, AstNode&& node) {
	nodes_.insert(nodes_.begin() + static_cast<ptrdiff_t>(first_child), std::move(node));
	// after the insert, the index of the first_child is now the index of the newly inserted parent node
	return first_child;
}

AstNode& Ast::get(Ast::NodeIndex index) {
	return nodes_[index];
}

void Ast::undo_nodes_from_lookahead(NodeIndex first) {
	nodes_.erase(nodes_.begin() + static_cast<ptrdiff_t>(first), nodes_.end());
}

} // namespace cero
