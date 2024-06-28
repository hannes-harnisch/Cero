#pragma once

#include "cero/io/Source.hpp"
#include "cero/syntax/AstNode.hpp"
#include "cero/syntax/AstVisitor.hpp"
#include "cero/syntax/TokenStream.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

/// Stores the abstract syntax tree for one source file. Contains no type information and is immutable. The underlying dynamic
/// array stores the AST nodes in pre-order.
class Ast {
public:
	/// Number of AST nodes.
	uint32_t num_nodes() const;

	/// Whether syntax errors were encountered during parsing.
	bool has_errors() const;

	/// Get a view of the underlying storage.
	std::span<const AstNode> raw() const;

	/// Creates a tree-like string representation of the AST.
	std::string to_string(const SourceGuard& source) const;

private:
	std::vector<AstNode> nodes_;
	bool has_errors_ = false;
	uint16_t current_num_children_ = 0;
	uint32_t current_num_descendants_ = 0;

	using NodeIndex = uint32_t;

	/// Reserves storage for the AST based on the number of tokens.
	explicit Ast(const TokenStream& token_stream);

	/// Stores a new node in the AST, positioning it as the rightmost child of the currently rightmost node. TODO: Not true
	NodeIndex store(AstNode&& node);

	/// Stores a new node in the AST as the parent of a node already in the AST. For performance reasons, should only be used
	/// when it cannot be known that a node must be the parent of an already stored node until after the child node was stored.
	NodeIndex store_parent_of(NodeIndex first_child, AstNode&& node);

	AstNode& get(NodeIndex index);

	NodeIndex next_index() const {
		return static_cast<NodeIndex>(nodes_.size());
	}

	void undo_nodes_from_lookahead(NodeIndex first);

	friend class Parser;
};

} // namespace cero
