#include "AstCompare.hpp"

#include <cero/syntax/Ast.hpp>
#include <doctest/doctest.h>

namespace tests {

AstCompare::AstCompare(const cero::Ast& ast) :
	cursor_(ast),
	current_level_(0) {
}

AstCompare::~AstCompare() {
	CHECK_EQ(current_level_, 0);
	CHECK(data_.empty());
}

void AstCompare::compare() {
	CHECK_EQ(current_level_, 0); // scopes were improperly closed if this isn't 0
	cursor_.visit_all(*this);
}

void AstCompare::root() {
	record(cero::AstNodeKind::Root);
}

void AstCompare::visit(const cero::AstRoot&) {
	expect(cero::AstNodeKind::Root);
}

void AstCompare::struct_definition(cero::AccessSpecifier access, std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::StructDefinition);

	data_.emplace(access);
	data_.emplace(name);
	record_children(cs);
}

void AstCompare::visit(const cero::AstStructDefinition& struct_def) {
	expect(cero::AstNodeKind::StructDefinition);

	auto access = pop<cero::AccessSpecifier>();
	CHECK_EQ(access, struct_def.access);

	auto name = pop<std::string_view>();
	CHECK_EQ(name, struct_def.name);
}

void AstCompare::enum_definition(cero::AccessSpecifier access, std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::EnumDefinition);

	data_.emplace(access);
	data_.emplace(name);
	record_children(cs);
}

void AstCompare::visit(const cero::AstEnumDefinition& enum_def) {
	expect(cero::AstNodeKind::EnumDefinition);

	auto access = pop<cero::AccessSpecifier>();
	CHECK_EQ(access, enum_def.access);

	auto name = pop<std::string_view>();
	CHECK_EQ(name, enum_def.name);
}

void AstCompare::function_definition(cero::AccessSpecifier access, std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::FunctionDefinition);

	data_.emplace(access);
	data_.emplace(name);
	record_children(cs);
}

void AstCompare::function_parameter(cero::ParameterSpecifier specifier, std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::FunctionParameter);

	data_.emplace(specifier);
	data_.emplace(name);
	record_children(cs);
}

void AstCompare::function_output(std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::FunctionOutput);

	data_.emplace(name);
	record_children(cs);
}

void AstCompare::visit(const cero::AstFunctionDefinition& func_def) {
	expect(cero::AstNodeKind::FunctionDefinition);

	auto access = pop<cero::AccessSpecifier>();
	CHECK_EQ(access, func_def.access);

	auto name = pop<std::string_view>();
	CHECK_EQ(name, func_def.name);

	visit_children(func_def.num_parameters);
	visit_children(func_def.num_outputs);
	visit_children(func_def.num_statements);
}

void AstCompare::visit(const cero::AstFunctionParameter& param) {
	expect(cero::AstNodeKind::FunctionParameter);

	auto specifier = pop<cero::ParameterSpecifier>();
	CHECK_EQ(specifier, param.specifier);

	auto param_name = pop<std::string_view>();
	CHECK_EQ(param_name, param.name);

	visit_child(); // type
	visit_child_if(param.has_default_argument);
}

void AstCompare::visit(const cero::AstFunctionOutput& output) {
	expect(cero::AstNodeKind::FunctionOutput);
	auto output_name = pop<std::string_view>();
	CHECK_EQ(output_name, output.name);

	visit_child(); // type
}

void AstCompare::block_statement(ChildScope cs) {
	record(cero::AstNodeKind::BlockStatement);
	record_children(cs);
}

void AstCompare::visit(const cero::AstBlockStatement& block_stmt) {
	expect(cero::AstNodeKind::BlockStatement);

	visit_children(block_stmt.num_statements);
}

void AstCompare::binding_statement(cero::BindingSpecifier specifier, std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::BindingStatement);

	data_.emplace(specifier);
	data_.emplace(name);
	record_children(cs);
}

void AstCompare::visit(const cero::AstBindingStatement& binding) {
	expect(cero::AstNodeKind::BindingStatement);

	auto specifier = pop<cero::BindingSpecifier>();
	CHECK_EQ(specifier, binding.specifier);

	auto param_name = pop<std::string_view>();
	CHECK_EQ(param_name, binding.name);

	visit_child_if(binding.has_type);
	visit_child_if(binding.has_initializer);
}

void AstCompare::visit(const cero::AstIfExpr& if_stmt) {
	expect(cero::AstNodeKind::IfExpr);

	visit_child();					  // condition
	visit_child();					  // then-statement
	visit_child_if(if_stmt.has_else); // else-statement
}

void AstCompare::while_loop(ChildScope cs) {
	record(cero::AstNodeKind::WhileLoop);
	record_children(cs);
}

void AstCompare::visit(const cero::AstWhileLoop& while_loop) {
	expect(cero::AstNodeKind::WhileLoop);

	visit_child();
	visit_children(while_loop.num_statements);
}

void AstCompare::visit(const cero::AstForLoop& for_loop) {
	expect(cero::AstNodeKind::ForLoop);

	visit_child();
	visit_child();
	visit_children(for_loop.num_statements);
}

void AstCompare::name_expr(std::string_view name) {
	record(cero::AstNodeKind::NameExpr);
	data_.emplace(name);
}

void AstCompare::generic_name_expr(std::string_view name, ChildScope cs) {
	record(cero::AstNodeKind::GenericNameExpr);

	data_.emplace(name);
	record_children(cs);
}

void AstCompare::visit(const cero::AstNameExpr& name_expr) {
	expect(cero::AstNodeKind::NameExpr);

	auto name = pop<std::string_view>();
	CHECK_EQ(name, name_expr.name);
}

void AstCompare::visit(const cero::AstGenericNameExpr& generic_name_expr) {
	expect(cero::AstNodeKind::GenericNameExpr);

	auto name = pop<std::string_view>();
	CHECK_EQ(name, generic_name_expr.name);

	visit_children(generic_name_expr.num_generic_args);
}

void AstCompare::member_expr(std::string_view name) {
	record(cero::AstNodeKind::MemberExpr);
	data_.emplace(name);
}

void AstCompare::visit(const cero::AstMemberExpr& member_expr) {
	expect(cero::AstNodeKind::MemberExpr);

	auto name = pop<std::string_view>();
	CHECK_EQ(name, member_expr.member);

	visit_child();
	visit_children(member_expr.num_generic_args);
}

void AstCompare::group_expr(ChildScope cs) {
	record(cero::AstNodeKind::GroupExpr);
	record_children(cs);
}

void AstCompare::visit(const cero::AstGroupExpr& group_expr) {
	expect(cero::AstNodeKind::GroupExpr);

	visit_children(group_expr.num_args);
}

void AstCompare::call_expr(ChildScope cs) {
	record(cero::AstNodeKind::CallExpr);
	record_children(cs);
}

void AstCompare::visit(const cero::AstCallExpr& call_expr) {
	expect(cero::AstNodeKind::CallExpr);

	visit_child();
	visit_children(call_expr.num_args);
}

void AstCompare::visit(const cero::AstIndexExpr& index_expr) {
	expect(cero::AstNodeKind::IndexExpr);

	visit_child();
	visit_children(index_expr.num_args);
}

void AstCompare::visit(const cero::AstArrayLiteralExpr& array_literal) {
	expect(cero::AstNodeKind::ArrayLiteralExpr);

	visit_children(array_literal.num_elements);
}

void AstCompare::unary_expr(cero::UnaryOperator op, ChildScope cs) {
	record(cero::AstNodeKind::UnaryExpr);
	data_.emplace(op);
	record_children(cs);
}

void AstCompare::visit(const cero::AstUnaryExpr& unary_expr) {
	expect(cero::AstNodeKind::UnaryExpr);

	auto op = pop<cero::UnaryOperator>();
	CHECK_EQ(op, unary_expr.op);

	visit_child();
}

void AstCompare::binary_expr(cero::BinaryOperator op, ChildScope cs) {
	record(cero::AstNodeKind::BinaryExpr);
	data_.emplace(op);
	record_children(cs);
}

void AstCompare::visit(const cero::AstBinaryExpr& binary_expr) {
	expect(cero::AstNodeKind::BinaryExpr);

	auto op = pop<cero::BinaryOperator>();
	CHECK_EQ(op, binary_expr.op);

	visit_child();
	visit_child();
}

void AstCompare::return_expr(ChildScope cs) {
	record(cero::AstNodeKind::ReturnExpr);
	record_children(cs);
}

void AstCompare::visit(const cero::AstReturnExpr& return_expr) {
	expect(cero::AstNodeKind::ReturnExpr);

	visit_children(return_expr.num_expressions);
}

void AstCompare::visit(const cero::AstThrowExpr& throw_expr) {
	expect(cero::AstNodeKind::ThrowExpr);

	visit_child_if(throw_expr.has_expression);
}

void AstCompare::visit(const cero::AstBreakExpr& break_expr) {
	expect(cero::AstNodeKind::BreakExpr);

	visit_child_if(break_expr.has_label);
}

void AstCompare::visit(const cero::AstContinueExpr& continue_expr) {
	expect(cero::AstNodeKind::ContinueExpr);

	visit_child_if(continue_expr.has_label);
}

void AstCompare::numeric_literal_expr(cero::NumericLiteralKind kind) {
	record(cero::AstNodeKind::NumericLiteralExpr);
	data_.emplace(kind);
}

void AstCompare::visit(const cero::AstNumericLiteralExpr& numeric_literal) {
	expect(cero::AstNodeKind::NumericLiteralExpr);

	auto kind = pop<cero::NumericLiteralKind>();
	CHECK_EQ(kind, numeric_literal.kind);
}

void AstCompare::visit(const cero::AstStringLiteralExpr& string_literal) {
	expect(cero::AstNodeKind::StringLiteralExpr);

	auto value = pop<std::string>();
	CHECK_EQ(value, string_literal.value);
}

void AstCompare::visit(const cero::AstPermissionExpr& permission) {
	expect(cero::AstNodeKind::PermissionExpr);

	auto specifier = pop<cero::PermissionSpecifier>();
	CHECK_EQ(specifier, permission.specifier);

	visit_children(permission.num_args);
}

void AstCompare::visit(const cero::AstPointerTypeExpr& ptr_type) {
	expect(cero::AstNodeKind::PointerTypeExpr);

	visit_child_if(ptr_type.has_permission);
	visit_child();
}

void AstCompare::visit(const cero::AstArrayTypeExpr& array_type) {
	expect(cero::AstNodeKind::ArrayTypeExpr);

	visit_child_if(array_type.has_bound);
	visit_child();
}

void AstCompare::visit(const cero::AstFunctionTypeExpr& func_type) {
	expect(cero::AstNodeKind::FunctionTypeExpr);

	visit_children(func_type.num_parameters);
	visit_children(func_type.num_outputs);
}

void AstCompare::visit_child() {
	++current_level_;
	cursor_.visit_child(*this);
	--current_level_;
}

void AstCompare::visit_child_if(bool condition) {
	if (condition) {
		visit_child();
	}
}

void AstCompare::visit_children(uint32_t n) {
	++current_level_;
	cursor_.visit_children(n, *this);
	--current_level_;
}

void AstCompare::expect(cero::AstNodeKind type) {
	auto recorded = pop<cero::AstNodeKind>();
	CHECK_EQ(recorded, type);

	auto level = pop<uint32_t>();
	CHECK_EQ(level, current_level_);
}

void AstCompare::record(cero::AstNodeKind type) {
	data_.emplace(type);
	data_.emplace(current_level_);
}

void AstCompare::record_children(ChildScope child_scope) {
	++current_level_;
	child_scope();
	--current_level_;
}

template<typename T>
T AstCompare::pop() {
	T* next = std::any_cast<T>(&data_.front());
	REQUIRE(next != nullptr);

	T value = std::move(*next);
	data_.pop();
	return value;
}

} // namespace tests
