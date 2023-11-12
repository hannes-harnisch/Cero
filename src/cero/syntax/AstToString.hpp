#pragma once

#include "cero/io/Source.hpp"
#include "cero/syntax/Ast.hpp"
#include "cero/syntax/AstVisitor.hpp"

namespace cero {

class AstToString : public AstVisitor {
	struct Edge {
		std::string_view branch;
		std::string_view prefix;
	};

	static constexpr Edge Body {"├── ", "│   "};
	static constexpr Edge Tail {"└── ", "    "};

	std::string string;
	const Ast& ast;
	const SourceLock& source;
	std::stack<std::string> prefixes;
	const Edge* edge = &Body;

public:
	AstToString(const Ast& ast, const SourceLock& source);

	std::string make_string();

private:
	void push_level();
	void pop_level();

	void set_tail(bool at_tail);
	void add_line(std::string_view text);
	void add_body_line(std::string_view text);
	void add_tail_line(std::string_view text);

	void visit(AstId node);
	void visit_body(AstId node);
	void visit_tail(AstId node);
	void visit_optional(OptionalAstId node);
	void visit_each_in(const auto& list);

	void visit(const AstRoot&) override;
	void visit(const AstStructDefinition&) override;
	void visit(const AstEnumDefinition&) override;
	void visit(const AstFunctionDefinition&) override;
	void visit(const AstFunctionDefinition::Parameter& parameter);
	void visit(const AstFunctionDefinition::Output& output);
	void visit(const AstBlockStatement&) override;
	void visit(const AstBindingStatement&) override;
	void visit(const AstIfExpr&) override;
	void visit(const AstWhileLoop&) override;
	void visit(const AstForLoop&) override;
	void visit(const AstNameExpr&) override;
	void visit(const AstGenericNameExpr&) override;
	void visit(const AstMemberExpr&) override;
	void visit(const AstGenericMemberExpr&) override;
	void visit(const AstGroupExpr&) override;
	void visit(const AstCallExpr&) override;
	void visit(const AstIndexExpr&) override;
	void visit(const AstArrayLiteralExpr&) override;
	void visit(const AstUnaryExpr&) override;
	void visit(const AstBinaryExpr&) override;
	void visit(const AstReturnExpr&) override;
	void visit(const AstThrowExpr&) override;
	void visit(const AstBreakExpr&) override;
	void visit(const AstContinueExpr&) override;
	void visit(const AstNumericLiteralExpr&) override;
	void visit(const AstStringLiteralExpr&) override;
	void visit(const AstVariabilityExpr&) override;
	void visit(const AstPointerTypeExpr&) override;
	void visit(const AstArrayTypeExpr&) override;
	void visit(const AstFunctionTypeExpr&) override;
	void visit(const AstFunctionTypeExpr::Parameter& parameter);
	void visit(const AstFunctionTypeExpr::Output& output);
};

} // namespace cero
