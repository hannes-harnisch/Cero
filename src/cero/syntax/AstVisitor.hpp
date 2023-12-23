#pragma once

#include "cero/syntax/AstNode.hpp"

namespace cero {

class AstVisitor {
public:
	virtual ~AstVisitor() = default;

	virtual void visit(const AstRoot& root) = 0;
	virtual void visit(const AstStructDefinition& struct_def) = 0;
	virtual void visit(const AstEnumDefinition& enum_def) = 0;
	virtual void visit(const AstFunctionDefinition& function_def) = 0;
	virtual void visit(const AstFunctionParameter& function_param) = 0;
	virtual void visit(const AstFunctionOutput& function_output) = 0;
	virtual void visit(const AstBlockStatement& block_stmt) = 0;
	virtual void visit(const AstBindingStatement& binding_stmt) = 0;
	virtual void visit(const AstIfExpr& if_stmt) = 0;
	virtual void visit(const AstWhileLoop& while_loop) = 0;
	virtual void visit(const AstForLoop& for_loop) = 0;
	virtual void visit(const AstNameExpr& name_expr) = 0;
	virtual void visit(const AstGenericNameExpr& generic_name) = 0;
	virtual void visit(const AstMemberExpr& member_expr) = 0;
	virtual void visit(const AstGenericMemberExpr& generic_member) = 0;
	virtual void visit(const AstGroupExpr& group_expr) = 0;
	virtual void visit(const AstCallExpr& call_expr) = 0;
	virtual void visit(const AstIndexExpr& index_expr) = 0;
	virtual void visit(const AstArrayLiteralExpr& array_literal) = 0;
	virtual void visit(const AstUnaryExpr& unary_expr) = 0;
	virtual void visit(const AstBinaryExpr& binary_expr) = 0;
	virtual void visit(const AstReturnExpr& return_expr) = 0;
	virtual void visit(const AstThrowExpr& throw_expr) = 0;
	virtual void visit(const AstBreakExpr& break_expr) = 0;
	virtual void visit(const AstContinueExpr& continue_expr) = 0;
	virtual void visit(const AstNumericLiteralExpr& numeric_literal) = 0;
	virtual void visit(const AstStringLiteralExpr& string_literal) = 0;
	virtual void visit(const AstVariabilityExpr& variability) = 0;
	virtual void visit(const AstPointerTypeExpr& pointer_type) = 0;
	virtual void visit(const AstArrayTypeExpr& array_type) = 0;
	virtual void visit(const AstFunctionTypeExpr& function_type) = 0;
};

} // namespace cero
