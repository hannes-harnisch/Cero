#pragma once

#include "driver/Source.hpp"
#include "syntax/AstNode.hpp"
#include "syntax/AstVisitor.hpp"

namespace cero {

class Ast {
public:
	void visit(AstVisitor& visitor) const;
	void visit_node(AstVisitor& visitor, AstId node_id) const;
	void visit_nodes(AstVisitor& visitor, AstIdSet node_ids) const;

	const AstNode&			 get(AstId id) const;
	std::span<const AstNode> get_multiple(AstIdSet ids) const;
	size_t					 get_node_count() const;

	const AstRoot& get_root() const;

	std::string to_string(const Source& source) const;
	void		log(const Source& source) const;

private:
	std::vector<AstNode> ast_nodes;

	AstId	  store(AstNode node);
	AstIdSet  store_multiple(std::span<AstNode> nodes);

	friend class Parser;
};

} // namespace cero