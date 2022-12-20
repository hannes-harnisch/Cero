#include "AstString.hpp"

#include "util/LookupTable.hpp"

namespace
{
	std::string_view to_string(ParameterKind kind)
	{
		using enum ParameterKind;
		switch (kind)
		{
			case In: return "in";
			case Let: return "let";
			case Var: return "var";
		}
		return {};
	}

	std::string_view to_string(Variability::Specifier spec)
	{
		using enum Variability::Specifier;
		switch (spec)
		{
			case In: return "in";
			case Var: return "var";
			case VarBounded: return "var (bounded)";
			case VarUnbounded: return "var (unbounded)";
		}
		return {};
	}

	std::string_view to_string(Binding::Specifier spec)
	{
		using enum Binding::Specifier;
		switch (spec)
		{
			case Let: return "let";
			case Var: return "var";
			case Const: return "const";
			case Static: return "static";
			case StaticVar: return "static var";
		}
		return {};
	}

	constexpr inline LookupTable<UnaryOperator, std::string_view> UNARY_OPERATOR_STRINGS = []
	{
		using enum UnaryOperator;

		LookupTable<UnaryOperator, std::string_view> t({});
		t[TryOperator]	 = "try";
		t[PreIncrement]	 = "prefix ++";
		t[PreDecrement]	 = "prefix --";
		t[PostIncrement] = "postfix ++";
		t[PostDecrement] = "postfix --";
		t[AddressOf]	 = "&";
		t[Dereference]	 = "^";
		t[Negation]		 = "-";
		t[LogicalNot]	 = "!";
		t[BitwiseNot]	 = "~";
		return t;
	}();

	constexpr inline LookupTable<BinaryOperator, std::string_view> BINARY_OPERATOR_STRINGS = []
	{
		using enum BinaryOperator;

		LookupTable<BinaryOperator, std::string_view> t({});
		t[Add]				= "+";
		t[Subtract]			= "-";
		t[Multiply]			= "*";
		t[Divide]			= "/";
		t[Remainder]		= "%";
		t[Power]			= "**";
		t[LogicalAnd]		= "&&";
		t[LogicalOr]		= "||";
		t[BitAnd]			= "&";
		t[BitOr]			= "|";
		t[Xor]				= "~";
		t[LeftShift]		= "<<";
		t[RightShift]		= ">>";
		t[Equality]			= "==";
		t[Inequality]		= "!=";
		t[Less]				= "<";
		t[Greater]			= ">";
		t[LessEqual]		= "<=";
		t[GreaterEqual]		= ">=";
		t[Assign]			= "=";
		t[AddAssign]		= "+=";
		t[SubtractAssign]	= "-=";
		t[MultiplyAssign]	= "*=";
		t[DivideAssign]		= "/=";
		t[RemainderAssign]	= "%=";
		t[PowerAssign]		= "**=";
		t[BitAndAssign]		= "&=";
		t[BitOrAssign]		= "|=";
		t[XorAssign]		= "~=";
		t[LeftShiftAssign]	= "<<=";
		t[RightShiftAssign] = ">>=";
		return t;
	}();
} // namespace

AstString::AstString(const SyntaxTree& ast, const Source& source) :
	string(std::format("Printing AST for {}\n", source.get_path())),
	ast(ast),
	source(source)
{
	prefixes.emplace();
}

std::string AstString::build()
{
	visit_each_in(ast.get_root_definitions());
	return std::move(string);
}

void AstString::push_level()
{
	auto prefix = prefixes.top();
	prefix.append(edge->prefix);
	prefixes.push(std::move(prefix));
}

void AstString::pop_level()
{
	prefixes.pop();
}

void AstString::set_tail(bool at_tail)
{
	edge = at_tail ? &TAIL : &BODY;
}

void AstString::add_line(std::string_view text)
{
	string.append(prefixes.top());
	string.append(edge->branch);
	string.append(text);
	string.append("\n");
}

void AstString::add_body_line(std::string_view text)
{
	set_tail(false);
	add_line(text);
}

void AstString::add_tail_line(std::string_view text)
{
	set_tail(true);
	add_line(text);
}

void AstString::visit_each_in(const auto& list)
{
	for (size_t i = 0; i != list.size(); ++i)
	{
		set_tail(i == list.size() - 1);
		visit(list[i]);
	}
}

void AstString::visit(Definition definition)
{
	auto& ast_node = ast.get(definition);
	std::visit([&](auto& node) { visit_node(node); }, ast_node);
}

void AstString::visit_node(const Function& function)
{
	add_line(std::format("function `{}`", function.name));
	push_level();

	add_body_line("parameters");
	push_level();
	visit_each_in(function.parameters);
	pop_level();

	add_body_line("return values");
	push_level();
	visit_each_in(function.returns);
	pop_level();

	add_tail_line("statements");
	push_level();
	visit_each_in(function.statements);
	pop_level();

	pop_level();
}

void AstString::visit(const Parameter& parameter)
{
	add_line(std::format("parameter `{}`", parameter.name));
	push_level();

	add_body_line(std::format("kind `{}`", to_string(parameter.kind)));

	bool has_default_argument = !parameter.default_argument.is_null();
	set_tail(!has_default_argument);
	add_line("type");
	push_level();
	visit_tail(parameter.type);
	pop_level();

	if (has_default_argument)
	{
		add_tail_line("default argument");
		push_level();
		visit_tail(parameter.default_argument.get());
		pop_level();
	}

	pop_level();
}

void AstString::visit(const ReturnValue& return_value)
{
	if (return_value.name.empty())
		add_line("return value");
	else
		add_line(std::format("return value `{}`", return_value.name));

	push_level();
	visit_tail(return_value.type);
	pop_level();
}

void AstString::visit_node(const Struct&)
{}

void AstString::visit_node(const Enum&)
{}

void AstString::visit(Expression expression)
{
	auto& ast_node = ast.get(expression);
	std::visit([&](auto& node) { visit_node(node); }, ast_node);
}

void AstString::visit_body(Expression expression)
{
	set_tail(false);
	visit(expression);
}

void AstString::visit_tail(Expression expression)
{
	set_tail(true);
	visit(expression);
}

void AstString::visit_optional(OptionalExpression optional_expression)
{
	if (optional_expression.is_null())
		return;

	push_level();
	visit_tail(optional_expression.get());
	pop_level();
}

void AstString::visit_node(Identifier id)
{
	add_line(std::format("identifier `{}`", id.name));
}

void AstString::visit_node(const GenericIdentifier& generic_id)
{
	add_line(std::format("generic identifier `{}`", generic_id.name));
	push_level();
	visit_each_in(generic_id.arguments);
	pop_level();
}

void AstString::visit_node(const Variability& variability)
{
	add_line(std::format("variability `{}`", to_string(variability.specifier)));
	push_level();
	visit_each_in(variability.arguments);
	pop_level();
}

void AstString::visit_node(const ArrayTypeExpression& array_type)
{
	add_line("array type");
	push_level();

	if (!array_type.count.is_null())
	{
		add_body_line("count");

		push_level();
		visit_tail(array_type.count.get());
		pop_level();
	}

	add_tail_line("element type");

	push_level();
	visit_tail(array_type.element_type);
	pop_level();

	pop_level();
}

void AstString::visit_node(const PointerTypeExpression& pointer_type)
{
	add_line("pointer type");
	push_level();

	set_tail(false);
	visit_node(pointer_type.variability);

	add_tail_line("type");

	push_level();
	visit_tail(pointer_type.type);
	pop_level();

	pop_level();
}

void AstString::visit_node(const NumericLiteral& numeric_literal)
{
	add_line(std::format("numeric literal `{}`", "oof"));
}

void AstString::visit_node(const StringLiteral& string_literal)
{
	add_line(std::format("string literal `{}`", string_literal.value));
}

void AstString::visit_node(const Binding& binding)
{
	add_line(std::format("{} binding `{}`", to_string(binding.specifier), binding.name));
	push_level();

	bool has_type = !binding.type.is_null();
	bool has_init = !binding.initializer.is_null();
	if (has_type)
	{
		set_tail(!has_init);
		add_line("type");
		push_level();
		visit_tail(binding.type.get());
		pop_level();
	}

	if (has_init)
	{
		add_tail_line("initializer");
		push_level();
		visit_tail(binding.initializer.get());
		pop_level();
	}

	pop_level();
}

void AstString::visit_node(const BlockExpression& block)
{
	add_line("block");
	push_level();
	visit_each_in(block.statements);
	pop_level();
}

void AstString::visit_node(const IfExpression& if_expression)
{
	add_line("if");
	push_level();

	add_body_line("condition");
	push_level();
	visit_tail(if_expression.condition);
	pop_level();

	bool has_else = !if_expression.else_expression.is_null();

	set_tail(!has_else);
	add_line("then");
	push_level();
	visit_tail(if_expression.then_expression);
	pop_level();

	if (has_else)
	{
		add_tail_line("else");
		push_level();
		visit_tail(if_expression.else_expression.get());
		pop_level();
	}

	pop_level();
}

void AstString::visit_node(const WhileLoop& while_loop)
{
	add_line("while");
	push_level();

	visit_body(while_loop.condition);
	visit_tail(while_loop.statement);

	pop_level();
}

void AstString::visit_node(const ForLoop& for_loop)
{
	add_line("for");
	push_level();

	visit_body(for_loop.binding);
	visit_body(for_loop.range_expression);
	visit_tail(for_loop.statement);

	pop_level();
}

void AstString::visit_node(const BreakExpression& break_expression)
{
	add_line("break");
	visit_optional(break_expression.label);
}

void AstString::visit_node(const ContinueExpression& continue_expression)
{
	add_line("continue");
	visit_optional(continue_expression.label);
}

void AstString::visit_node(const ReturnExpression& return_expression)
{
	add_line("return");
	visit_optional(return_expression.expression);
}

void AstString::visit_node(const ThrowExpression& throw_expression)
{
	add_line("throw");
	visit_optional(throw_expression.expression);
}

void AstString::visit_node(const MemberAccess& member_access)
{
	add_line("member access");
	push_level();

	visit_body(member_access.target);
	add_tail_line(member_access.member);

	pop_level();
}

void AstString::visit_node(const CallExpression& call)
{
	bool has_callee = !call.callee.is_null();
	if (!has_callee && call.arguments.size() == 1)
	{
		visit(call.arguments[0]);
		return;
	}

	add_line("call expression");
	push_level();

	if (has_callee)
		visit_body(call.callee.get());

	add_tail_line("arguments");
	push_level();
	visit_each_in(call.arguments);
	pop_level();

	pop_level();
}

void AstString::visit_node(const IndexExpression& index)
{
	add_line("index expression");
	push_level();

	visit_body(index.target);

	add_tail_line("arguments");
	push_level();
	visit_each_in(index.arguments);
	pop_level();

	pop_level();
}

void AstString::visit_node(const UnaryExpression& unary_expression)
{
	add_line(std::format("`{}`", UNARY_OPERATOR_STRINGS[unary_expression.op]));
	push_level();

	visit_tail(unary_expression.operand);

	pop_level();
}

void AstString::visit_node(const BinaryExpression& binary_expression)
{
	add_line(std::format("`{}`", BINARY_OPERATOR_STRINGS[binary_expression.op]));
	push_level();

	visit_body(binary_expression.left);
	visit_tail(binary_expression.right);

	pop_level();
}
