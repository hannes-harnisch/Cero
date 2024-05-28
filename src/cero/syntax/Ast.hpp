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
	/// Gets the number of AST nodes.
	uint32_t num_nodes() const;

	/// Whether syntax errors were encountered during parsing.
	bool has_errors() const;

	/// Get a view of the underlying storage.
	std::span<const AstNode> raw() const;

	/// Creates a tree-like string representation of the AST.
	std::string to_string(const SourceGuard& source) const;

private:
	std::vector<AstNode> nodes_;
	bool has_errors_;

	/// Index of the leftmost descendant of a node (the node that is syntactically "first")
	using NodeIndex = uint32_t;

	explicit Ast(const TokenStream& token_stream);

	template<typename T>
	T& store() {
		// valid thanks to pointer interconvertibility for unions and union members
		return reinterpret_cast<T&>(nodes_.emplace_back(AstNode(T())));
	}

	void insert_parent(NodeIndex first_descendant_index, AstNode&& node);

	NodeIndex next_index() const;

	friend class Parser;
};

} // namespace cero
