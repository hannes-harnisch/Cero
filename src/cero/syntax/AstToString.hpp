#pragma once

#include "cero/io/Source.hpp"
#include "cero/syntax/AstCursor.hpp"
#include "cero/syntax/AstVisitor.hpp"

namespace cero {

class AstToString : public AstVisitor {
public:
	AstToString(const Ast& ast, const SourceGuard& source);

	std::string make_string() &&;

private:
	struct Edge {
		std::string_view branch;
		std::string_view prefix;
	};

	static constexpr Edge Body {"├── ", "│   "};
	static constexpr Edge Tail {"└── ", "    "};

	AstCursor cursor_;
	const SourceGuard& source_;
	std::stack<std::string> prefixes_;
	const Edge* edge_;
	std::string string_;

	template<typename T>
	std::string locate(const T& t) const;

	void push_level();
	void pop_level();

	void set_tail(bool at_tail);
	void add_line(std::string_view text);
	void add_body_line(std::string_view text);
	void add_tail_line(std::string_view text);

	void visit_child_at_body();
	void visit_child_at_tail();
	void visit_child();
	void visit_child_if(bool condition);
	void visit_children(uint32_t n);

	void visit(const AstRoot& root) override;
	void visit(const AstStructDefinition& struct_def) override;
	void visit(const AstEnumDefinition& enum_def) override;
	void visit(const AstFunctionDefinition& func_def) override;
	void visit(const AstFunctionParameter& param) override;
	void visit(const AstFunctionOutput& output) override;
	void visit(const AstBlockStatement& block_stmt) override;
	void visit(const AstBindingStatement& binding) override;
	void visit(const AstIfExpr& if_stmt) override;
	void visit(const AstWhileLoop& while_loop) override;
	void visit(const AstForLoop& for_loop) override;
	void visit(const AstNameExpr& name_expr) override;
	void visit(const AstGenericNameExpr& generic_name_expr) override;
	void visit(const AstMemberExpr& member_expr) override;
	void visit(const AstGroupExpr& group_expr) override;
	void visit(const AstCallExpr& call_expr) override;
	void visit(const AstIndexExpr& index_expr) override;
	void visit(const AstArrayLiteralExpr& array_literal) override;
	void visit(const AstUnaryExpr& unary_expr) override;
	void visit(const AstBinaryExpr& binary_expr) override;
	void visit(const AstReturnExpr& return_expr) override;
	void visit(const AstThrowExpr& throw_expr) override;
	void visit(const AstBreakExpr& break_expr) override;
	void visit(const AstContinueExpr& continue_expr) override;
	void visit(const AstNumericLiteralExpr& numeric_literal) override;
	void visit(const AstStringLiteralExpr& string_literal) override;
	void visit(const AstPermissionExpr& permission) override;
	void visit(const AstPointerTypeExpr& ptr_type) override;
	void visit(const AstArrayTypeExpr& array_type) override;
	void visit(const AstFunctionTypeExpr& func_type) override;
};

} // namespace cero
