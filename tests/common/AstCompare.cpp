#include "AstCompare.hpp"

#include <doctest/doctest.h>
#include <syntax/Ast.hpp>

AstChildScope::~AstChildScope() {
	--level;
}

AstChildScope::AstChildScope(uint32_t& level) :
	level(level) {
	++level;
}

AstCompare::AstCompare(const cero::Ast& ast) :
	ast(ast),
	current_level(0) {
}

AstCompare::~AstCompare() {
	CHECK(current_level == 0);
	CHECK(data.empty());
}

void AstCompare::compare() {
	CHECK(current_level == 0); // scopes were improperly closed if this isn't 0
	ast.visit(*this);
}

AstChildScope AstCompare::mark_children() {
	return AstChildScope(current_level);
}

void AstCompare::add_root() {
	record(cero::AstNodeKind::Root);
}

void AstCompare::visit(const cero::AstRoot& root) {
	expect(cero::AstNodeKind::Root);

	ast.visit_nodes(*this, root.definitions);
}

void AstCompare::add_struct_definition(cero::AccessSpecifier access, std::string_view name) {
	record(cero::AstNodeKind::StructDefinition);

	data.emplace(access);
	data.emplace(name);
}

void AstCompare::visit(const cero::AstStructDefinition& struct_def) {
	expect(cero::AstNodeKind::StructDefinition);

	auto access = pop<cero::AccessSpecifier>();
	CHECK(access == struct_def.access);

	auto name = pop<std::string_view>();
	CHECK(name == struct_def.name);
}

void AstCompare::add_enum_definition(cero::AccessSpecifier access, std::string_view name) {
	record(cero::AstNodeKind::EnumDefinition);

	data.emplace(access);
	data.emplace(name);
}

void AstCompare::visit(const cero::AstEnumDefinition& enum_def) {
	expect(cero::AstNodeKind::EnumDefinition);

	auto access = pop<cero::AccessSpecifier>();
	CHECK(access == enum_def.access);

	auto name = pop<std::string_view>();
	CHECK(name == enum_def.name);
}

void AstCompare::add_function_definition(cero::AccessSpecifier access, std::string_view name) {
	record(cero::AstNodeKind::FunctionDefinition);
	data.emplace(access);
	data.emplace(name);
}

void AstCompare::add_function_definition_parameter(cero::ParameterSpecifier specifier, std::string_view name) {
	data.emplace(specifier);
	data.emplace(name);
}

void AstCompare::add_function_definition_output(std::string_view name) {
	data.emplace(name);
}

void AstCompare::visit(const cero::AstFunctionDefinition& function_def) {
	expect(cero::AstNodeKind::FunctionDefinition);

	auto access = pop<cero::AccessSpecifier>();
	CHECK(access == function_def.access);

	auto name = pop<std::string_view>();
	CHECK(name == function_def.name);

	for (auto& param : function_def.parameters) {
		auto specifier = pop<cero::ParameterSpecifier>();
		CHECK(specifier == param.specifier);

		auto param_name = pop<std::string_view>();
		CHECK(param_name == param.name);

		visit_child(param.type);

		if (auto default_arg = param.default_argument)
			visit_child(default_arg.get());
	}

	for (auto& output : function_def.outputs) {
		auto output_name = pop<std::string_view>();
		CHECK(output_name == output.name);

		visit_child(output.type);
	}

	visit_children(function_def.statements);
}

void AstCompare::add_block_statement() {
	record(cero::AstNodeKind::BlockStatement);
}

void AstCompare::visit(const cero::AstBlockStatement& block_stmt) {
	expect(cero::AstNodeKind::BlockStatement);

	visit_children(block_stmt.statements);
}

void AstCompare::add_binding_statement(cero::BindingSpecifier specifier, std::string_view name) {
	record(cero::AstNodeKind::BindingStatement);

	data.emplace(specifier);
	data.emplace(name);
}

void AstCompare::visit(const cero::AstBindingStatement& binding_stmt) {
	expect(cero::AstNodeKind::BindingStatement);

	auto specifier = pop<cero::BindingSpecifier>();
	CHECK(specifier == binding_stmt.specifier);

	auto param_name = pop<std::string_view>();
	CHECK(param_name == binding_stmt.name);

	if (auto type = binding_stmt.type)
		visit_child(type.get());

	if (auto init = binding_stmt.initializer)
		visit_child(init.get());
}

void AstCompare::visit(const cero::AstIfStatement& if_stmt) {
	expect(cero::AstNodeKind::IfStatement);

	visit_child(if_stmt.condition);
	visit_child(if_stmt.then_expression);

	if (auto else_expr = if_stmt.else_expression)
		visit_child(else_expr.get());
}

void AstCompare::add_while_loop() {
	record(cero::AstNodeKind::WhileLoop);
}

void AstCompare::visit(const cero::AstWhileLoop& while_loop) {
	expect(cero::AstNodeKind::WhileLoop);

	visit_child(while_loop.condition);
	visit_child(while_loop.statement);
}

void AstCompare::visit(const cero::AstForLoop& for_loop) {
	expect(cero::AstNodeKind::ForLoop);

	visit_child(for_loop.binding);
	visit_child(for_loop.range_expression);
	visit_child(for_loop.statement);
}

void AstCompare::add_identifier_expr(std::string_view name) {
	record(cero::AstNodeKind::IdentifierExpr);
	data.emplace(name);
}

void AstCompare::visit(const cero::AstIdentifierExpr& identifier) {
	expect(cero::AstNodeKind::IdentifierExpr);

	auto name = pop<std::string_view>();
	CHECK(name == identifier.name);
}

void AstCompare::add_generic_identifier_expr(std::string_view name) {
	record(cero::AstNodeKind::GenericIdentifierExpr);
	data.emplace(name);
}

void AstCompare::visit(const cero::AstGenericIdentifierExpr& generic_identifier) {
	expect(cero::AstNodeKind::GenericIdentifierExpr);

	auto name = pop<std::string_view>();
	CHECK(name == generic_identifier.name);

	visit_children(generic_identifier.arguments);
}

void AstCompare::add_member_expr(std::string_view name) {
	record(cero::AstNodeKind::MemberExpr);
	data.emplace(name);
}

void AstCompare::visit(const cero::AstMemberExpr& member_expr) {
	expect(cero::AstNodeKind::MemberExpr);

	auto name = pop<std::string_view>();
	CHECK(name == member_expr.member);

	visit_child(member_expr.target);
}

void AstCompare::add_group_expr() {
	record(cero::AstNodeKind::GroupExpr);
}

void AstCompare::visit(const cero::AstGroupExpr& group) {
	expect(cero::AstNodeKind::GroupExpr);

	visit_children(group.arguments);
}

void AstCompare::add_call_expr() {
	record(cero::AstNodeKind::CallExpr);
}

void AstCompare::visit(const cero::AstCallExpr& call_expr) {
	expect(cero::AstNodeKind::CallExpr);

	visit_child(call_expr.callee);
	visit_children(call_expr.arguments);
}

void AstCompare::visit(const cero::AstIndexExpr& index_expr) {
	expect(cero::AstNodeKind::IndexExpr);

	visit_child(index_expr.target);
	visit_children(index_expr.arguments);
}

void AstCompare::visit(const cero::AstArrayLiteralExpr& array_literal) {
	expect(cero::AstNodeKind::ArrayLiteralExpr);

	visit_children(array_literal.elements);
}

void AstCompare::add_unary_expr(cero::UnaryOperator op) {
	record(cero::AstNodeKind::UnaryExpr);
	data.emplace(op);
}

void AstCompare::visit(const cero::AstUnaryExpr& unary_expr) {
	expect(cero::AstNodeKind::UnaryExpr);

	auto op = pop<cero::UnaryOperator>();
	CHECK(op == unary_expr.op);

	visit_child(unary_expr.operand);
}

void AstCompare::add_binary_expr(cero::BinaryOperator op) {
	record(cero::AstNodeKind::BinaryExpr);
	data.emplace(op);
}

void AstCompare::visit(const cero::AstBinaryExpr& binary_expr) {
	expect(cero::AstNodeKind::BinaryExpr);

	auto op = pop<cero::BinaryOperator>();
	CHECK(op == binary_expr.op);

	visit_child(binary_expr.left);
	visit_child(binary_expr.right);
}

void AstCompare::add_return_expr() {
	record(cero::AstNodeKind::ReturnExpr);
}

void AstCompare::visit(const cero::AstReturnExpr& return_expr) {
	expect(cero::AstNodeKind::ReturnExpr);

	if (auto expr = return_expr.expression)
		visit_child(expr.get());
}

void AstCompare::visit(const cero::AstThrowExpr& throw_expr) {
	expect(cero::AstNodeKind::ThrowExpr);

	if (auto expr = throw_expr.expression)
		visit_child(expr.get());
}

void AstCompare::visit(const cero::AstBreakExpr& break_expr) {
	expect(cero::AstNodeKind::BreakExpr);

	if (auto label = break_expr.label)
		visit_child(label.get());
}

void AstCompare::visit(const cero::AstContinueExpr& continue_expr) {
	expect(cero::AstNodeKind::ContinueExpr);

	if (auto label = continue_expr.label)
		visit_child(label.get());
}

void AstCompare::add_numeric_literal_expr(cero::NumericLiteralKind kind) {
	record(cero::AstNodeKind::NumericLiteralExpr);
	data.emplace(kind);
}

void AstCompare::visit(const cero::AstNumericLiteralExpr& numeric_literal) {
	expect(cero::AstNodeKind::NumericLiteralExpr);

	auto kind = pop<cero::NumericLiteralKind>();
	CHECK(kind == numeric_literal.kind);
}

void AstCompare::visit(const cero::AstStringLiteralExpr& string_literal) {
	expect(cero::AstNodeKind::StringLiteralExpr);

	auto value = pop<std::string>();
	CHECK(value == string_literal.value);
}

void AstCompare::visit(const cero::AstVariabilityExpr& variability) {
	expect(cero::AstNodeKind::VariabilityExpr);

	auto specifier = pop<cero::VariabilitySpecifier>();
	CHECK(specifier == variability.specifier);

	visit_children(variability.arguments);
}

void AstCompare::visit(const cero::AstPointerTypeExpr& pointer_type) {
	expect(cero::AstNodeKind::PointerTypeExpr);

	auto specifier = pop<cero::VariabilitySpecifier>();
	CHECK(specifier == pointer_type.variability.specifier);

	visit_children(pointer_type.variability.arguments);

	visit_child(pointer_type.type);
}

void AstCompare::visit(const cero::AstArrayTypeExpr& array_type) {
	expect(cero::AstNodeKind::ArrayTypeExpr);

	if (auto bound = array_type.bound)
		visit_child(bound.get());

	visit_child(array_type.element_type);
}

void AstCompare::visit(const cero::AstFunctionTypeExpr& function_type) {
	expect(cero::AstNodeKind::FunctionTypeExpr);

	for (auto& param : function_type.parameters) {
		auto specifier = pop<cero::ParameterSpecifier>();
		CHECK(specifier == param.specifier);

		auto param_name = pop<std::string_view>();
		CHECK(param_name == param.name);

		visit_child(param.type);
	}

	for (auto& output : function_type.outputs) {
		auto output_name = pop<std::string_view>();
		CHECK(output_name == output.name);

		visit_child(output.type);
	}
}

void AstCompare::visit_child(cero::AstId id) {
	++current_level;
	ast.visit_node(*this, id);
	--current_level;
}

void AstCompare::visit_children(cero::AstIdSet ids) {
	++current_level;
	ast.visit_nodes(*this, ids);
	--current_level;
}

void AstCompare::record(cero::AstNodeKind type) {
	data.emplace(type);
	data.emplace(current_level);
}

void AstCompare::expect(cero::AstNodeKind type) {
	auto recorded = pop<cero::AstNodeKind>();
	CHECK(recorded == type);

	auto level = pop<uint32_t>();
	CHECK(level == current_level);
}

template<typename T>
T AstCompare::pop() {
	T* next = std::any_cast<T>(&data.front());
	REQUIRE(next != nullptr);

	T value = std::move(*next);
	data.pop();
	return value;
}
