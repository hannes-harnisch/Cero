#pragma once

#include "util/Macros.hpp"
#include "util/Traits.hpp"

namespace cero {

class AstId {
	uint32_t index;

	explicit AstId(uint32_t index) :
		index(index) {
	}

	friend class Ast;
	friend class OptionalAstId;
	friend class AstIdSet;
};

class OptionalAstId {
public:
	OptionalAstId() :
		index(UINT32_MAX) {
	}

	OptionalAstId(AstId id) :
		index(id.index) {
	}

	explicit operator bool() const {
		return index != UINT32_MAX;
	}

	bool is_null() const {
		return index == UINT32_MAX;
	}

	AstId get() const {
		CERO_ASSERT_DEBUG(index != UINT32_MAX, "can't call get on a null AST ID");
		return AstId(index);
	}

private:
	uint32_t index;
};

class AstIdSet {
public:
	AstIdSet() :
		count(0),
		first(UINT32_MAX) {
	}

	AstId operator[](size_t index) const {
		if (index < count)
			return AstId(first + static_cast<uint32_t>(index));

		fail_assert("index out of bounds");
	}

	uint32_t size() const {
		return count;
	}

private:
	uint32_t count;
	uint32_t first;

	AstIdSet(uint32_t count, uint32_t first) :
		count(count),
		first(first) {
	}

	friend class Ast;
};

using StringId = std::string_view;

struct AstRoot {
	AstIdSet definitions;
};

enum class AccessSpecifier : uint8_t {
	None,
	Private,
	Public
};

struct AstStructDefinition {
	AccessSpecifier access;
	StringId name;
};

struct AstEnumDefinition {
	AccessSpecifier access;
	StringId name;
};

enum class ParameterSpecifier : uint8_t {
	None,
	In,
	Var
};

struct AstFunctionDefinition {
	struct Parameter {
		ParameterSpecifier specifier = {};
		AstId type;
		StringId name;
		OptionalAstId default_argument;
	};

	struct Output {
		AstId type;
		StringId name;
	};

	AccessSpecifier access = {};
	StringId name;
	std::vector<Parameter> parameters;
	std::vector<Output> outputs;
	AstIdSet statements;
};

struct AstBlockStatement {
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
	BindingSpecifier specifier = {};
	OptionalAstId type;
	StringId name;
	OptionalAstId initializer;
};

struct AstIfExpr {
	AstId condition;
	AstId then_expression;
	OptionalAstId else_expression;
};

struct AstWhileLoop {
	AstId condition;
	AstId statement;
};

struct AstForLoop {
	AstId binding;
	AstId range_expression;
	AstId statement;
};

struct AstNameExpr {
	StringId name;
};

struct AstGenericNameExpr {
	StringId name;
	AstIdSet arguments;
};

struct AstMemberExpr {
	AstId target;
	StringId member;
};

struct AstGenericMemberExpr {
	AstId target;
	StringId member;
	AstIdSet arguments;
};

struct AstGroupExpr {
	AstIdSet arguments;
};

struct AstCallExpr {
	AstId callee;
	AstIdSet arguments;
};

struct AstIndexExpr {
	AstId target;
	AstIdSet arguments;
};

struct AstArrayLiteralExpr {
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

std::string_view to_string(UnaryOperator op);

struct AstUnaryExpr {
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

std::string_view to_string(BinaryOperator op);

struct AstBinaryExpr {
	BinaryOperator op;
	AstId left;
	AstId right;
};

struct AstReturnExpr {
	AstIdSet return_values;
};

struct AstThrowExpr {
	OptionalAstId expression;
};

struct AstBreakExpr {
	OptionalAstId label;
};

struct AstContinueExpr {
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
	NumericLiteralKind kind = {};
};

struct AstStringLiteralExpr {
	std::string value;
};

enum class VariabilitySpecifier : uint8_t {
	In,
	Var,
	VarBounded,
	VarUnbounded
};

struct AstVariabilityExpr {
	VariabilitySpecifier specifier = {};
	AstIdSet arguments;
};

struct AstPointerTypeExpr {
	AstVariabilityExpr variability;
	AstId type;
};

struct AstArrayTypeExpr {
	OptionalAstId bound;
	AstId element_type;
};

struct AstFunctionTypeExpr {
	struct Parameter {
		ParameterSpecifier specifier = {};
		AstId type;
		StringId name;
	};

	struct Output {
		AstId type;
		StringId name;
	};

	std::vector<Parameter> parameters;
	std::vector<Output> outputs;
};

#define CERO_AST_NODE_TYPES                                                                                                    \
	CERO_AST_NODE_TYPE(Root)                                                                                                   \
	CERO_AST_NODE_TYPE(StructDefinition)                                                                                       \
	CERO_AST_NODE_TYPE(EnumDefinition)                                                                                         \
	CERO_AST_NODE_TYPE(FunctionDefinition)                                                                                     \
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

enum class AstNodeKind : uint8_t {
#define CERO_AST_NODE_TYPE(X) X,
	CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
};

class AstNode {
public:
	AstNodeKind get_type() const {
		return type;
	}

	template<typename T>
	const T& as() const {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	if constexpr (std::is_same_v<T, Ast##X>) {                                                                                 \
		if (type == AstNodeKind::X)                                                                                            \
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
		if (type == AstNodeKind::X)                                                                                            \
			return &X##_;                                                                                                      \
		else                                                                                                                   \
			return nullptr;                                                                                                    \
	} else
		CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		{
			static_assert(always_false<T>, "T must be an AST node type.");
		}
	}

	~AstNode() {
		switch (type) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: X##_.~Ast##X(); break;
			CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		}
	}

	AstNode(AstNode&& other) noexcept :
		type(other.type) {
		switch (type) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: new (&X##_) Ast##X(std::move(other.X##_)); break;
			CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		}
	}

	AstNode& operator=(AstNode&& other) noexcept {
		type = other.type;
		switch (type) {
#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	case AstNodeKind::X: X##_ = std::move(other.X##_); break;
			CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
		}
		return *this;
	}

private:
	AstNodeKind type;

	union {
#define CERO_AST_NODE_TYPE(X) Ast##X X##_;
		CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE
	};

#define CERO_AST_NODE_TYPE(X)                                                                                                  \
	AstNode(Ast##X node) :                                                                                                     \
		type(AstNodeKind::X) {                                                                                                 \
		new (&X##_) Ast##X(std::move(node));                                                                                   \
	}
	CERO_AST_NODE_TYPES
#undef CERO_AST_NODE_TYPE

	friend class Parser;
	friend class Ast;
};

} // namespace cero
