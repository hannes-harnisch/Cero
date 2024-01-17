#pragma once

#include <cero/syntax/AstCursor.hpp>
#include <cero/syntax/AstVisitor.hpp>

#include <any>
#include <queue>

class [[nodiscard]] AstChildScope {
public:
	~AstChildScope();

private:
	uint32_t& level_;

	friend class AstCompare;
	AstChildScope(uint32_t& level);
};

class AstCompare : public cero::AstVisitor {
public:
	AstCompare(const cero::Ast& ast);
	~AstCompare() override;

	void compare();

	AstChildScope mark_children();

	void add_root();
	void add_struct_definition(cero::AccessSpecifier access, std::string_view name);
	void add_enum_definition(cero::AccessSpecifier access, std::string_view name);
	void add_function_definition(cero::AccessSpecifier access, std::string_view name);
	void add_function_parameter(cero::ParameterSpecifier specifier, std::string_view name);
	void add_function_output(std::string_view name);
	void add_block_statement();
	void add_binding_statement(cero::BindingSpecifier specifier, std::string_view name);
	void add_while_loop();
	void add_name_expr(std::string_view name);
	void add_member_expr(std::string_view name);
	void add_group_expr();
	void add_call_expr();
	void add_unary_expr(cero::UnaryOperator op);
	void add_binary_expr(cero::BinaryOperator op);
	void add_return_expr();
	void add_numeric_literal_expr(cero::NumericLiteralKind kind);

	AstCompare(const AstCompare&) = delete;
	AstCompare& operator=(const AstCompare&) = delete;

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
	void visit_children(uint16_t n);

	void record(cero::AstNodeKind type);
	void expect(cero::AstNodeKind type);

	template<typename T>
	T pop();
};
