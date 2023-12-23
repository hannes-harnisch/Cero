#pragma once

#include <cero/syntax/Ast.hpp>
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
	void add_generic_name_expr(std::string_view name);
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
	const cero::Ast& ast_;
	std::queue<std::any> data_;
	uint32_t current_level_;

	void visit(const cero::AstRoot& root) override;
	void visit(const cero::AstStructDefinition& struct_def) override;
	void visit(const cero::AstEnumDefinition& enum_def) override;
	void visit(const cero::AstFunctionDefinition& function_def) override;
	void visit(const cero::AstFunctionParameter& function_param) override;
	void visit(const cero::AstFunctionOutput& function_output) override;
	void visit(const cero::AstBlockStatement& block_stmt) override;
	void visit(const cero::AstBindingStatement& binding_stmt) override;
	void visit(const cero::AstIfExpr& if_stmt) override;
	void visit(const cero::AstWhileLoop& while_loop) override;
	void visit(const cero::AstForLoop& for_loop) override;
	void visit(const cero::AstNameExpr& name_expr) override;
	void visit(const cero::AstGenericNameExpr& generic_name) override;
	void visit(const cero::AstMemberExpr& member_expr) override;
	void visit(const cero::AstGenericMemberExpr& generic_member) override;
	void visit(const cero::AstGroupExpr& group) override;
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
	void visit(const cero::AstVariabilityExpr& variability) override;
	void visit(const cero::AstPointerTypeExpr& pointer_type) override;
	void visit(const cero::AstArrayTypeExpr& array_type) override;
	void visit(const cero::AstFunctionTypeExpr& function_type) override;

	void visit_child(cero::AstId id);
	void visit_children(cero::AstIdSet ids);

	void record(cero::AstNodeKind type);
	void expect(cero::AstNodeKind type);

	template<typename T>
	T pop();
};
