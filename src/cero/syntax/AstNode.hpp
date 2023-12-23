#pragma once

#include "cero/io/Source.hpp"
#include "cero/util/Macros.hpp"
#include "cero/util/Traits.hpp"

namespace cero {

class AstId {
	uint32_t index_;

	explicit AstId(uint32_t index) :
		index_(index) {
	}

	friend class Ast;
	friend class OptionalAstId;
	friend class AstIdSet;
};

class OptionalAstId {
public:
	OptionalAstId() :
		index_(UINT32_MAX) {
	}

	OptionalAstId(AstId id) :
		index_(id.index_) {
	}

	explicit operator bool() const {
		return index_ != UINT32_MAX;
	}

	bool is_null() const {
		return index_ == UINT32_MAX;
	}

	AstId get() const {
		CERO_ASSERT_DEBUG(index_ != UINT32_MAX, "can't call get on a null AST ID");
		return AstId(index_);
	}

private:
	uint32_t index_;
};

class AstIdSet {
public:
	AstIdSet() :
		count_(0),
		first_(UINT32_MAX) {
	}

	AstId operator[](size_t index) const {
		if (index < count_) {
			return AstId(first_ + static_cast<uint32_t>(index));
		}

		fail_assert("index out of bounds");
	}

	uint32_t size() const {
		return count_;
	}

private:
	uint32_t count_;
	uint32_t first_;

	AstIdSet(uint32_t count, uint32_t first) :
		count_(count),
		first_(first) {
	}

	friend class Ast;
};

using StringId = std::string_view;

#define CERO_AST_NODE_TYPES                                                                                                    \
	CERO_AST_NODE_TYPE(Root)                                                                                                   \
	CERO_AST_NODE_TYPE(StructDefinition)                                                                                       \
	CERO_AST_NODE_TYPE(EnumDefinition)                                                                                         \
	CERO_AST_NODE_TYPE(FunctionDefinition)                                                                                     \
	CERO_AST_NODE_TYPE(FunctionParameter)                                                                                      \
	CERO_AST_NODE_TYPE(FunctionOutput)                                                                                         \
	CERO_AST_NODE_TYPE(BlockStatement)                                                                                         \
	CERO_AST_NODE_TYPE(BindingStatement)                                                                                       \
	CERO_AST_NODE_TYPE(IfExpr)                                                                                                 \
	CERO_AST_NODE_TYPE(WhileLoop)                                                                                              \
	CERO_AST_NODE_TYPE(ForLoop)                                                                                                \
	CERO_AST_NODE_TYPE(NameExpr)                                                                                               \
	CERO_AST_NODE_TYPE(GenericNameExpr)                                                                                        \
	CERO_AST_NODE_TYPE(MemberExpr)                                                                                             \
	CERO_AST_NODE_TYPE(GenericMemberExpr)                                                                                      \
	CERO_AST_NODE_TYPE(GroupExpr)                                                                                              \
	CERO_AST_NODE_TYPE(CallExpr)                                                                                               \
	CERO_AST_NODE_TYPE(IndexExpr)                                                                                              \
	CERO_AST_NODE_TYPE(ArrayLiteralExpr)                                                                                       \
	CERO_AST_NODE_TYPE(UnaryExpr)                                                                                              \
	CERO_AST_NODE_TYPE(BinaryExpr)                                                                                             \
	CERO_AST_NODE_TYPE(ReturnExpr)                                                                                             \
	CERO_AST_NODE_TYPE(ThrowExpr)                                                                                              \
	CERO_AST_NODE_TYPE(BreakExpr)                                                                                              \
	CERO_AST_NODE_TYPE(ContinueExpr)                                                                                           \
	CERO_AST_NODE_TYPE(NumericLiteralExpr)                                                                                     \
	CERO_AST_NODE_TYPE(StringLiteralExpr)                                                                                      \
	CERO_AST_NODE_TYPE(VariabilityExpr)                                                                                        \
	CERO_AST_NODE_TYPE(PointerTypeExpr)                                                                                        \
	CERO_AST_NODE_TYPE(ArrayTypeExpr)                                                                                          \
	CERO_AST_NODE_TYPE(FunctionTypeExpr)

enum class AstNodeKind {
#define CERO_AST_NODE_TYPE(X) X,
	CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
};

template<AstNodeKind K>
struct AstNodeHeader {
	AstNodeKind kind : 8;
	unsigned offset : SourceOffsetBits;

	AstNodeHeader(uint32_t offset) :
		kind(K),
		offset(offset) {
	}
};

struct AstRoot {
	AstNodeHeader<AstNodeKind::Root> header;
	AstIdSet definitions;
};

enum class AccessSpecifier : uint8_t {
	None,
	Private,
	Public
};

struct AstStructDefinition {
	AstNodeHeader<AstNodeKind::StructDefinition> header;
	AccessSpecifier access;
	StringId name;
};

struct AstEnumDefinition {
	AstNodeHeader<AstNodeKind::EnumDefinition> header;
	AccessSpecifier access;
	StringId name;
};

enum class ParameterSpecifier : uint8_t {
	None,
	In,
	Var
};

struct AstFunctionParameter {
	AstNodeHeader<AstNodeKind::FunctionParameter> header;
	ParameterSpecifier specifier = {};
	AstId type;
	StringId name;
	OptionalAstId default_argument;
};

struct AstFunctionOutput {
	AstNodeHeader<AstNodeKind::FunctionOutput> header;
	AstId type;
	StringId name;
};

struct AstFunctionDefinition {
	AstNodeHeader<AstNodeKind::FunctionDefinition> header;
	AccessSpecifier access = {};
	StringId name;
	std::vector<AstFunctionParameter> parameters;
	std::vector<AstFunctionOutput> outputs;
	AstIdSet statements;
};

struct AstBlockStatement {
	AstNodeHeader<AstNodeKind::BlockStatement> header;
	AstIdSet statements;
};

enum class BindingSpecifier : uint8_t {
	Let,
	Var,
	Const,
	Static,
	StaticVar
};

struct AstBindingStatement {
	AstNodeHeader<AstNodeKind::BindingStatement> header;
	BindingSpecifier specifier = {};
	OptionalAstId type;
	StringId name;
	OptionalAstId initializer;
};

struct AstIfExpr {
	AstNodeHeader<AstNodeKind::IfExpr> header;
	AstId condition;
	AstId then_expression;
	OptionalAstId else_expression;
};

struct AstWhileLoop {
	AstNodeHeader<AstNodeKind::WhileLoop> header;
	AstId condition;
	AstId statement;
};

struct AstForLoop {
	AstNodeHeader<AstNodeKind::ForLoop> header;
	AstId binding;
	AstId range_expression;
	AstId statement;
};

struct AstNameExpr {
	AstNodeHeader<AstNodeKind::NameExpr> header;
	StringId name;
};

struct AstGenericNameExpr {
	AstNodeHeader<AstNodeKind::GenericNameExpr> header;
	StringId name;
	AstIdSet arguments;
};

struct AstMemberExpr {
	AstNodeHeader<AstNodeKind::MemberExpr> header;
	AstId target;
	StringId member;
};

struct AstGenericMemberExpr {
	AstNodeHeader<AstNodeKind::GenericMemberExpr> header;
	AstId target;
	StringId member;
	AstIdSet arguments;
};

struct AstGroupExpr {
	AstNodeHeader<AstNodeKind::GroupExpr> header;
	AstIdSet arguments;
};

struct AstCallExpr {
	AstNodeHeader<AstNodeKind::CallExpr> header;
	AstId callee;
	AstIdSet arguments;
};

struct AstIndexExpr {
	AstNodeHeader<AstNodeKind::IndexExpr> header;
	AstId target;
	AstIdSet arguments;
};

struct AstArrayLiteralExpr {
	AstNodeHeader<AstNodeKind::ArrayLiteralExpr> header;
	AstIdSet elements;
};

enum class UnaryOperator : uint8_t {
	PreIncrement,
	PreDecrement,
	PostIncrement,
	PostDecrement,
	AddressOf,
	Dereference,
	Negate,
	LogicalNot,
	BitwiseNot
};

std::string_view unary_operator_to_string(UnaryOperator op);

struct AstUnaryExpr {
	AstNodeHeader<AstNodeKind::UnaryExpr> header;
	UnaryOperator op;
	AstId operand;
};

enum class BinaryOperator : uint8_t {
	Add,
	Subtract,
	Multiply,
	Divide,
	Remainder,
	Power,
	LogicalAnd,
	LogicalOr,
	BitAnd,
	BitOr,
	Xor,
	LeftShift,
	RightShift,
	Equal,
	NotEqual,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Assign,
	AddAssign,
	SubtractAssign,
	MultiplyAssign,
	DivideAssign,
	RemainderAssign,
	PowerAssign,
	AndAssign,
	OrAssign,
	XorAssign,
	LeftShiftAssign,
	RightShiftAssign
};

std::string_view binary_operator_to_string(BinaryOperator op);

struct AstBinaryExpr {
	AstNodeHeader<AstNodeKind::BinaryExpr> header;
	BinaryOperator op;
	AstId left;
	AstId right;
};

struct AstReturnExpr {
	AstNodeHeader<AstNodeKind::ReturnExpr> header;
	AstIdSet return_values;
};

struct AstThrowExpr {
	AstNodeHeader<AstNodeKind::ThrowExpr> header;
	OptionalAstId expression;
};

struct AstBreakExpr {
	AstNodeHeader<AstNodeKind::BreakExpr> header;
	OptionalAstId label;
};

struct AstContinueExpr {
	AstNodeHeader<AstNodeKind::ContinueExpr> header;
	OptionalAstId label;
};

enum class NumericLiteralKind : uint8_t {
	Decimal,
	Hexadecimal,
	Binary,
	Octal,
	Float,
	Character
};

struct AstNumericLiteralExpr {
	AstNodeHeader<AstNodeKind::NumericLiteralExpr> header;
	NumericLiteralKind kind = {};
};

struct AstStringLiteralExpr {
	AstNodeHeader<AstNodeKind::StringLiteralExpr> header;
	std::string value;
};

enum class VariabilitySpecifier : uint8_t {
	In,
	Var,
	VarBounded,
	VarUnbounded
};

struct AstVariabilityExpr {
	AstNodeHeader<AstNodeKind::VariabilityExpr> header;
	VariabilitySpecifier specifier = {};
	AstIdSet arguments;
};

struct AstPointerTypeExpr {
	AstNodeHeader<AstNodeKind::PointerTypeExpr> header;
	AstVariabilityExpr variability;
	AstId type;
};

struct AstArrayTypeExpr {
	AstNodeHeader<AstNodeKind::ArrayTypeExpr> header;
	OptionalAstId bound;
	AstId element_type;
};

struct AstFunctionTypeExpr {
	AstNodeHeader<AstNodeKind::FunctionTypeExpr> header;
	std::vector<AstFunctionParameter> parameters;
	std::vector<AstFunctionOutput> outputs;
};

class AstNode {
public:
	AstNodeKind get_kind() const {
		return Root_.header.kind;
	}

	uint32_t get_offset() const {
		return Root_.header.offset;
	}

	template<typename T>
	const T& as() const {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		if (get_kind() == AstNodeKind::X)                                                                                      \
			return X##_;                                                                                                       \
                                                                                                                               \
		fail_assert("node does not hold expected type");                                                                       \
	} else
		CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	template<typename T>
	const T* get() const {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		return get_kind() == AstNodeKind::X ? &X##_ : nullptr;                                                                 \
	} else
		CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	~AstNode() {
		switch (get_kind()) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: X##_.~Ast##X(); break;
			CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		}
	}

	AstNode(AstNode&& other) noexcept {
		switch (other.get_kind()) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: new (&X##_) Ast##X(std::move(other.X##_)); break;
			CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		}
	}

	AstNode& operator=(AstNode&& other) noexcept {
		switch (other.get_kind()) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: X##_ = std::move(other.X##_); break;
			CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		}
		return *this;
	}

private:
	union {
#define CERO_AST_NODE_TYPE(X) Ast##X X##_;
		CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
	};

#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	AstNode(Ast##X&& node) :                                                                                                   \
		X##_(std::move(node)) {                                                                                                \
	}
	CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE

	friend class Parser;
	friend class Ast;
};

} // namespace cero
