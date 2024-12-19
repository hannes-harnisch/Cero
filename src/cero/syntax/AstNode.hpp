#pragma once

#include "cero/io/Source.hpp"
#include "cero/syntax/AstNodeKind.hpp"
#include "cero/syntax/AstVisitor.hpp"
#include "cero/util/Macros.hpp"
#include "cero/util/Traits.hpp"

namespace cero {

union AstNode {
public:
	AstNodeKind get_kind() const {
		return Root_.header.kind;
	}

	uint32_t get_offset() const {
		return Root_.header.offset;
	}

	uint32_t num_children() const {
		switch (Root_.header.kind) {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	case AstNodeKind::X: return X##_.num_children();
			CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		}
		fail_unreachable();
	}

	void visit(AstVisitor& visitor) const {
		switch (Root_.header.kind) {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	case AstNodeKind::X: visitor.visit(X##_); break;
			CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		}
	}

	template<typename T>
	T& as() {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		if (Root_.header.kind == AstNodeKind::X) return X##_;                                                                  \
                                                                                                                               \
		fail_check("node does not hold expected type");                                                                        \
	} else
		CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	template<typename T>
	const T& as() const {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		if (Root_.header.kind == AstNodeKind::X) return X##_;                                                                  \
                                                                                                                               \
		fail_check("node does not hold expected type");                                                                        \
	} else
		CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	template<typename T>
	T* get() {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		return Root_.header.kind == AstNodeKind::X ? &X##_ : nullptr;                                                          \
	} else
		CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	template<typename T>
	const T* get() const {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		return Root_.header.kind == AstNodeKind::X ? &X##_ : nullptr;                                                          \
	} else
		CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	~AstNode() {
		switch (Root_.header.kind) {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	case AstNodeKind::X: X##_.~Ast##X(); break;
			CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		}
	}

	AstNode(AstNode&& other) noexcept {
		switch (other.Root_.header.kind) {
#define CERO_AST_NODE_KIND(X)                                                                                                  \
	case AstNodeKind::X: new (&X##_) Ast##X(std::move(other.X##_)); break;
			CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND
		}
	}

	AstNode& operator=(AstNode&& other) noexcept {
		this->~AstNode();
		new (this) AstNode(std::move(other));
		return *this;
	}

private:
#define CERO_AST_NODE_KIND(X) Ast##X X##_;
	CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND

#define CERO_AST_NODE_KIND(X)                                                                                                  \
	AstNode(Ast##X&& node) :                                                                                                   \
		X##_(std::move(node)) {                                                                                                \
	}
	CERO_AST_NODE_KINDS
#undef CERO_AST_NODE_KIND

	friend class Parser;
	friend class Ast;
};

} // namespace cero
