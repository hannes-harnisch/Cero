#pragma once

#include <cero/syntax/AstCursor.hpp>
#include <cero/syntax/AstVisitor.hpp>
#include <cero/util/FunctionRef.hpp>

#include <any>
#include <queue>

namespace tests {

class AstCompare : public cero::AstVisitor {
public:
	using ChildScope = cero::FunctionRef<void()>;

	AstCompare(const cero::Ast& ast);
	~AstCompare() override;

	// Perform the comparison.
	void compare();

	void root();
	void struct_definition(cero::AccessSpecifier access, std::string_view name, ChildScope cs);
	void enum_definition(cero::AccessSpecifier access, std::string_view name, ChildScope cs);
	void function_definition(cero::AccessSpecifier access, std::string_view name, ChildScope cs);
	void function_parameter(cero::ParameterSpecifier specifier, std::string_view name, ChildScope cs);
	void function_output(std::string_view name, ChildScope cs);
	void block_statement(ChildScope cs);
	void binding_statement(cero::BindingSpecifier specifier, std::string_view name, ChildScope cs);
	void while_loop(ChildScope cs);
	void name_expr(std::string_view name);
	void generic_name_expr(std::string_view name, ChildScope cs);
	void member_expr(std::string_view name);
	void group_expr(ChildScope cs);
	void call_expr(ChildScope cs);
	void unary_expr(cero::UnaryOperator op, ChildScope cs);
	void binary_expr(cero::BinaryOperator op, ChildScope cs);
	void return_expr(ChildScope cs);
	void numeric_literal_expr(cero::NumericLiteralKind kind);

	AstCompare(AstCompare&&) = delete;
	AstCompare& operator=(AstCompare&&) = delete;

private:
	cero::AstCursor cursor_;
	std::queue<std::any> data_;
	uint32_t current_level_;

	void visit(const cero::AstRoot& root) override;
	void visit(const cero::AstStructDefinition& struct_def) override;
	void visit(const cero::AstEnumDefinition& enum_def) override;
	void visit(const cero::AstFunctionDefinition& func_def) override;
	void visit(const cero::AstFunctionParameter& param) override;
	void visit(const cero::AstFunctionOutput& output) override;
	void visit(const cero::AstBlockStatement& block_stmt) override;
	void visit(const cero::AstBindingStatement& binding) override;
	void visit(const cero::AstIfExpr& if_stmt) override;
	void visit(const cero::AstWhileLoop& while_loop) override;
	void visit(const cero::AstForLoop& for_loop) override;
	void visit(const cero::AstNameExpr& name_expr) override;
	void visit(const cero::AstGenericNameExpr& generic_name_expr) override;
	void visit(const cero::AstMemberExpr& member_expr) override;
	void visit(const cero::AstGroupExpr& group_expr) override;
	void visit(const cero::AstCallExpr& call_expr) override;
	void visit(const cero::AstIndexExpr& index_expr) override;
	void visit(const cero::AstArrayLiteralExpr& array_literal) override;
	void visit(const cero::AstUnaryExpr& unary_expr) override;
	void visit(const cero::AstBinaryExpr& binary_expr) override;
	void visit(const cero::AstReturnExpr& return_expr) override;
	void visit(const cero::AstThrowExpr& throw_expr) override;
	void visit(const cero::AstBreakExpr& break_expr) override;
	void visit(const cero::AstContinueExpr& continue_expr) override;
	void visit(const cero::AstNumericLiteralExpr& numeric_literal) override;
	void visit(const cero::AstStringLiteralExpr& string_literal) override;
	void visit(const cero::AstPermissionExpr& permission) override;
	void visit(const cero::AstPointerTypeExpr& ptr_type) override;
	void visit(const cero::AstArrayTypeExpr& array_type) override;
	void visit(const cero::AstFunctionTypeExpr& func_type) override;

	void visit_child();
	void visit_child_if(bool condition);
	void visit_children(uint32_t n);

	void expect(cero::AstNodeKind type);
	void record(cero::AstNodeKind type);
	void record_children(ChildScope cs);

	template<typename T>
	T pop();
};

} // namespace tests
