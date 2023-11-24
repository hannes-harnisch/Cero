#include "Ast.hpp"

#include "cero/syntax/AstToString.hpp"

namespace cero {

void Ast::visit(AstVisitor& visitor) const {
	visit_node(visitor, AstId(static_cast<uint32_t>(ast_nodes_.size()) - 1));
}

void Ast::visit_node(AstVisitor& visitor, AstId node_id) const {
	auto& node = ast_nodes_[node_id.index_];
	switch (node.type) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: visitor.visit(node.X##_); break;
		CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
	}
}

void Ast::visit_nodes(AstVisitor& visitor, AstIdSet ids) const {
	const auto end = ids.first_ + ids.count_;
	for (auto i = ids.first_; i != end; ++i) {
		visit_node(visitor, AstId(i));
	}
}

const AstNode& Ast::get(AstId id) const {
	return ast_nodes_[id.index_];
}

std::span<const AstNode> Ast::get_multiple(AstIdSet ids) const {
	return std::span(ast_nodes_.data() + ids.first_, ids.count_);
}

size_t Ast::get_node_count() const {
	return ast_nodes_.size();
}

const AstRoot& Ast::get_root() const {
	return ast_nodes_.back().Root_;
}

std::string Ast::to_string(const SourceLock& source) const {
	AstToString ast_to_string(*this, source);
	return ast_to_string.make_string();
}

AstId Ast::store(AstNode node) {
	const uint32_t index = static_cast<uint32_t>(ast_nodes_.size());
	ast_nodes_.emplace_back(std::move(node));
	return AstId(index);
}

AstIdSet Ast::store_multiple(std::span<AstNode> nodes) {
	const uint32_t index = static_cast<uint32_t>(ast_nodes_.size());
	ast_nodes_.insert(ast_nodes_.end(), std::make_move_iterator(nodes.begin()), std::make_move_iterator(nodes.end()));
	return AstIdSet(static_cast<uint32_t>(nodes.size()), index);
}

} // namespace cero
