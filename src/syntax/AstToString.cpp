#include "AstToString.hpp"

#include "util/Fail.hpp"

namespace cero {

namespace {

	std::string_view to_string(ParameterSpecifier specifier) {
		using enum ParameterSpecifier;
		switch (specifier) {
			case None: return "value";
			case In: return "in";
			case Var: return "var";
		}
		fail_unreachable();
	}

	std::string_view to_string(VariabilitySpecifier spec) {
		using enum VariabilitySpecifier;
		switch (spec) {
			case In: return "in";
			case Var: return "var";
			case VarBounded: return "var (bounded)";
			case VarUnbounded: return "var (unbounded)";
		}
		fail_unreachable();
	}

	std::string_view to_string(BindingSpecifier spec) {
		using enum BindingSpecifier;
		switch (spec) {
			case Let: return "let";
			case Var: return "var";
			case Const: return "const";
			case Static: return "static";
			case StaticVar: return "static var";
		}
		fail_unreachable();
	}

	std::string_view to_string(NumericLiteralKind kind) {
		using enum NumericLiteralKind;
		switch (kind) {
			case Decimal: return "decimal";
			case Hexadecimal: return "hexadecimal";
			case Binary: return "binary";
			case Octal: return "octal";
			case Float: return "float";
			case Character: return "character";
		}
		fail_unreachable();
	}

} // namespace

AstToString::AstToString(const Ast& ast, const Source& source) :
	string(std::format("Printing AST for {}\n", source.get_path())),
	ast(ast),
	source(source) {
	prefixes.emplace();
}

std::string AstToString::make_string() {
	ast.visit(*this);
	return std::move(string);
}

void AstToString::push_level() {
	std::string prefix = prefixes.top();
	prefix.append(edge->prefix);
	prefixes.push(std::move(prefix));
}

void AstToString::pop_level() {
	prefixes.pop();
}

void AstToString::set_tail(bool at_tail) {
	edge = at_tail ? &Tail : &Body;
}

void AstToString::add_line(std::string_view text) {
	string.append(prefixes.top());
	string.append(edge->branch);
	string.append(text);
	string.append("\n");
}

void AstToString::add_body_line(std::string_view text) {
	set_tail(false);
	add_line(text);
}

void AstToString::add_tail_line(std::string_view text) {
	set_tail(true);
	add_line(text);
}

void AstToString::visit(AstId node) {
	ast.visit_node(*this, node);
}

void AstToString::visit_body(AstId node) {
	set_tail(false);
	visit(node);
}

void AstToString::visit_tail(AstId node) {
	set_tail(true);
	visit(node);
}

void AstToString::visit_optional(OptionalAstId node) {
	if (node.is_null())
		return;

	push_level();
	visit_tail(node.get());
	pop_level();
}

void AstToString::visit_each_in(const auto& list) {
	for (size_t i = 0; i != list.size(); ++i) {
		set_tail(i == list.size() - 1);
		visit(list[i]);
	}
}

void AstToString::visit(const AstRoot& root) {
	visit_each_in(root.definitions);
}

void AstToString::visit(const AstStructDefinition& struct_def) {
	add_line(std::format("struct `{}`", struct_def.name));
	push_level();

	pop_level();
}

void AstToString::visit(const AstEnumDefinition& enum_def) {
	add_line(std::format("enum `{}`", enum_def.name));
	push_level();

	pop_level();
}

void AstToString::visit(const AstFunctionDefinition& function_def) {
	add_line(std::format("function `{}`", function_def.name));
	push_level();

	add_body_line("parameters");
	push_level();
	visit_each_in(function_def.parameters);
	pop_level();

	add_body_line("outputs");
	push_level();
	visit_each_in(function_def.outputs);
	pop_level();

	add_tail_line("statements");
	push_level();
	visit_each_in(function_def.statements);
	pop_level();

	pop_level();
}

void AstToString::visit(const AstFunctionDefinition::Parameter& parameter) {
	add_line(std::format("{} parameter `{}`", to_string(parameter.specifier), parameter.name));
	push_level();

	bool has_default_argument = !parameter.default_argument.is_null();
	set_tail(!has_default_argument);
	visit(parameter.type);

	if (has_default_argument)
		visit_tail(parameter.default_argument.get());

	pop_level();
}

void AstToString::visit(const AstFunctionDefinition::Output& output) {
	if (output.name.empty())
		add_line("output");
	else
		add_line(std::format("output `{}`", output.name));

	push_level();
	visit_tail(output.type);
	pop_level();
}

void AstToString::visit(const AstBlockStatement& block) {
	add_line("block");
	push_level();
	visit_each_in(block.statements);
	pop_level();
}

void AstToString::visit(const AstBindingStatement& binding) {
	add_line(std::format("{} binding `{}`", to_string(binding.specifier), binding.name));
	push_level();

	bool has_type = !binding.type.is_null();
	bool has_init = !binding.initializer.is_null();
	if (has_type) {
		set_tail(!has_init);
		add_line("type");
		push_level();
		visit_tail(binding.type.get());
		pop_level();
	}

	if (has_init) {
		add_tail_line("initializer");
		push_level();
		visit_tail(binding.initializer.get());
		pop_level();
	}

	pop_level();
}

void AstToString::visit(const AstIfStatement& if_stmt) {
	add_line("if");
	push_level();

	add_body_line("condition");
	push_level();
	visit_tail(if_stmt.condition);
	pop_level();

	bool has_else = !if_stmt.else_expression.is_null();

	set_tail(!has_else);
	add_line("then");
	push_level();
	visit_tail(if_stmt.then_expression);
	pop_level();

	if (has_else) {
		add_tail_line("else");
		push_level();
		visit_tail(if_stmt.else_expression.get());
		pop_level();
	}

	pop_level();
}

void AstToString::visit(const AstWhileLoop& while_loop) {
	add_line("while");
	push_level();

	visit_body(while_loop.condition);
	visit_tail(while_loop.statement);

	pop_level();
}

void AstToString::visit(const AstForLoop& for_loop) {
	add_line("for");
	push_level();

	visit_body(for_loop.binding);
	visit_body(for_loop.range_expression);
	visit_tail(for_loop.statement);

	pop_level();
}

void AstToString::visit(const AstIdentifierExpr& identifier) {
	add_line(std::format("identifier `{}`", identifier.name));
}

void AstToString::visit(const AstGenericIdentifierExpr& generic_identifier) {
	add_line(std::format("generic identifier `{}`", generic_identifier.name));
	push_level();
	visit_each_in(generic_identifier.arguments);
	pop_level();
}

void AstToString::visit(const AstMemberExpr& member_expr) {
	add_line("member expression");
	push_level();

	visit_body(member_expr.target);
	add_tail_line(member_expr.member);

	pop_level();
}

void AstToString::visit(const AstGroupExpr& group) {
	if (group.arguments.size() == 1) {
		visit(group.arguments[0]);
		return;
	}

	add_line("group expression");
	push_level();
	visit_each_in(group.arguments);
	pop_level();
}

void AstToString::visit(const AstCallExpr& call) {
	add_line("call expression");
	push_level();

	visit_body(call.callee);

	add_tail_line("arguments");
	push_level();
	visit_each_in(call.arguments);
	pop_level();

	pop_level();
}

void AstToString::visit(const AstIndexExpr& index) {
	add_line("index expression");
	push_level();

	visit_body(index.target);

	add_tail_line("arguments");
	push_level();
	visit_each_in(index.arguments);
	pop_level();

	pop_level();
}

void AstToString::visit(const AstArrayLiteralExpr& array_literal) {
	add_line("array literal");

	push_level();
	visit_each_in(array_literal.elements);
	pop_level();
}

void AstToString::visit(const AstUnaryExpr& unary_expression) {
	add_line(std::format("`{}`", to_string(unary_expression.op)));
	push_level();

	visit_tail(unary_expression.operand);

	pop_level();
}

void AstToString::visit(const AstBinaryExpr& binary_expression) {
	add_line(std::format("`{}`", to_string(binary_expression.op)));
	push_level();

	visit_body(binary_expression.left);
	visit_tail(binary_expression.right);

	pop_level();
}

void AstToString::visit(const AstReturnExpr& return_expression) {
	add_line("return");
	visit_optional(return_expression.expression);
}

void AstToString::visit(const AstThrowExpr& throw_expression) {
	add_line("throw");
	visit_optional(throw_expression.expression);
}

void AstToString::visit(const AstBreakExpr& break_expression) {
	add_line("break");
	visit_optional(break_expression.label);
}

void AstToString::visit(const AstContinueExpr& continue_expression) {
	add_line("continue");
	visit_optional(continue_expression.label);
}

void AstToString::visit(const AstNumericLiteralExpr& numeric_literal) {
	add_line(std::format("{} literal `{}`", to_string(numeric_literal.kind), " ---TODO--- ")); // TODO: add number
}

void AstToString::visit(const AstStringLiteralExpr& string_literal) {
	add_line(std::format("string literal `{}`", string_literal.value));
}

void AstToString::visit(const AstVariabilityExpr& variability) {
	add_line(std::format("variability `{}`", to_string(variability.specifier)));
	push_level();
	visit_each_in(variability.arguments);
	pop_level();
}

void AstToString::visit(const AstPointerTypeExpr& pointer_type) {
	add_line("pointer type");
	push_level();

	set_tail(false);
	visit(pointer_type.variability);

	add_tail_line("type");

	push_level();
	visit_tail(pointer_type.type);
	pop_level();

	pop_level();
}

void AstToString::visit(const AstArrayTypeExpr& array_type) {
	add_line("array type");
	push_level();

	if (auto bound = array_type.bound) {
		add_body_line("count");

		push_level();
		visit_tail(bound.get());
		pop_level();
	}

	add_tail_line("element type");

	push_level();
	visit_tail(array_type.element_type);
	pop_level();

	pop_level();
}

void AstToString::visit(const AstFunctionTypeExpr& function_type) {
	add_line("function type");
	push_level();

	add_body_line("parameters");
	push_level();
	visit_each_in(function_type.parameters);
	pop_level();

	add_tail_line("return values");
	push_level();
	visit_each_in(function_type.outputs);
	pop_level();

	pop_level();
}

void AstToString::visit(const AstFunctionTypeExpr::Parameter& parameter) {
	add_line(std::format("{} parameter `{}`", to_string(parameter.specifier), parameter.name));

	push_level();
	visit_tail(parameter.type);
	pop_level();
}

void AstToString::visit(const AstFunctionTypeExpr::Output& output) {
	if (output.name.empty())
		add_line("output");
	else
		add_line(std::format("output `{}`", output.name));

	push_level();
	visit_tail(output.type);
	pop_level();
}

} // namespace cero