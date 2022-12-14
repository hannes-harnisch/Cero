#include "AstString.hpp"

#include <magic_enum.hpp>

AstString::AstString(const SyntaxTree& ast, const Source& source) :
	string(std::format("Printing AST for {}\n", source.get_path())),
	ast(ast),
	source(source)
{
	prefixes.emplace();
}

std::string AstString::build()
{
	auto definitions = ast.get_root_definitions();
	for (size_t i = 0; i != definitions.size(); ++i)
	{
		set_edge(definitions, i);
		visit(definitions[i]);
	}
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

void AstString::set_edge(const auto& range, size_t index)
{
	set_tail(index == range.size() - 1);
}

void AstString::add_line(std::string text)
{
	string.append(prefixes.top());
	string.append(edge->branch);
	string.append(text);
	string.append("\n");
}

void AstString::visit(Definition definition)
{
	auto& ast_node = ast.get(definition);
	std::visit([&](auto& node) { visit(node); }, ast_node);
}

void AstString::visit(const Function& function)
{
	add_line(std::format("function `{}`", function.name));
	push_level();

	set_tail(false);
	add_line("parameters");
	push_level();
	for (size_t i = 0; i != function.parameters.size(); ++i)
	{
		set_edge(function.parameters, i);
		visit(function.parameters[i]);
	}
	pop_level();

	set_tail(false);
	add_line("return values");
	push_level();
	for (size_t i = 0; i != function.returns.size(); ++i)
	{
		set_edge(function.returns, i);
		visit(function.returns[i]);
	}
	pop_level();

	set_tail(true);
	add_line("statements");
	push_level();
	for (size_t i = 0; i != function.statements.size(); ++i)
	{
		set_edge(function.statements, i);
		visit(function.statements[i]);
	}
	pop_level();

	pop_level();
}

void AstString::visit(const Parameter& parameter)
{
	add_line(std::format("parameter `{}`", parameter.name));
	push_level();

	set_tail(false);
	add_line(std::format("kind `{}`", magic_enum::enum_name(parameter.kind)));

	bool has_default_argument = !parameter.default_argument.is_null();

	set_tail(!has_default_argument);
	add_line("type");
	push_level();
	set_tail(true);
	visit(parameter.type);
	pop_level();

	if (has_default_argument)
	{
		set_tail(true);
		add_line("default argument");
		push_level();
		visit(parameter.default_argument);
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
	set_tail(true);
	visit(return_value.type);
	pop_level();
}

void AstString::visit(const Struct& struct_node)
{}

void AstString::visit(const Enum& enum_node)
{}

void AstString::visit(Expression expression)
{
	auto& ast_node = ast.get(expression);
	std::visit([&](auto& node) { visit(node); }, ast_node);
}

void AstString::visit(const BinaryExpression& binary_expression)
{
	push_level();

	set_tail(false);
	visit(binary_expression.left);

	set_tail(true);
	visit(binary_expression.right);

	pop_level();
}

void AstString::visit(const GenericIdentifier&)
{}

void AstString::visit(const ArrayTypeExpression&)
{}

void AstString::visit(const PointerTypeExpression&)
{}

void AstString::visit(const NumericLiteral&)
{}

void AstString::visit(const StringLiteral&)
{}

void AstString::visit(const LetBinding&)
{}

void AstString::visit(const VarBinding&)
{}

void AstString::visit(const BlockExpression&)
{}

void AstString::visit(const GroupExpression&)
{}

void AstString::visit(const IfExpression&)
{}

void AstString::visit(const WhileLoop&)
{}

void AstString::visit(const ForLoop&)
{}

void AstString::visit(const Access& access)
{}

void AstString::visit(const Call& call)
{}

void AstString::visit(const Indexing& indexing)
{}

void AstString::visit(const BreakExpression& break_expression)
{}

void AstString::visit(const ContinueExpression& continue_expression)
{}

void AstString::visit(const ReturnExpression& return_expression)
{}

void AstString::visit(const ThrowExpression& throw_expression)
{}

void AstString::visit(const TryExpression& try_expression)
{}

void AstString::visit(const PreIncrement& pre_increment)
{}

void AstString::visit(const PreDecrement& pre_decrement)
{}

void AstString::visit(const PostIncrement& post_increment)
{}

void AstString::visit(const PostDecrement& post_decrement)
{}

void AstString::visit(const AddressOf& address_of)
{}

void AstString::visit(const Dereference& dereference)
{}

void AstString::visit(const Negation& negation)
{}

void AstString::visit(const LogicalNot& logical_not)
{}

void AstString::visit(const BitwiseNot& bitwise_not)
{}

void AstString::visit(const Addition& addition)
{}

void AstString::visit(const Subtraction& subtraction)
{}

void AstString::visit(const Multiplication& multiplication)
{}

void AstString::visit(const Division& division)
{}

void AstString::visit(const Remainder& remainder)
{}

void AstString::visit(const Exponentiation& exponentiation)
{}

void AstString::visit(const LogicalAnd& logical_and)
{}

void AstString::visit(const LogicalOr& logical_or)
{}

void AstString::visit(const BitwiseAnd& bitwise_and)
{}

void AstString::visit(const BitwiseOr& bitwise_or)
{}

void AstString::visit(const Xor& xor_expression)
{}

void AstString::visit(const LeftShift& left_shift)
{}

void AstString::visit(const RightShift& right_shift)
{}

void AstString::visit(const Equality& equality)
{}

void AstString::visit(const Inequality& inequality)
{}

void AstString::visit(const Less& less)
{}

void AstString::visit(const Greater& greater)
{}

void AstString::visit(const LessEqual& less_equal)
{}

void AstString::visit(const GreaterEqual& greater_equal)
{}

void AstString::visit(const Assignment& assignment)
{}

void AstString::visit(const AdditionAssignment& addition_assignment)
{}

void AstString::visit(const SubtractionAssignment& subtraction_assignment)
{}

void AstString::visit(const MultiplicationAssignment& multiplication_assignment)
{}

void AstString::visit(const DivisionAssignment& division_assignment)
{}

void AstString::visit(const RemainderAssignment& remainder_assignment)
{}

void AstString::visit(const ExponentiationAssignment& exponentiation_assignment)
{}

void AstString::visit(const BitwiseAndAssignment& bitwise_and_assignment)
{}

void AstString::visit(const BitwiseOrAssignment& bitwise_or_assignment)
{}

void AstString::visit(const XorAssignment& xor_assignment)
{}

void AstString::visit(const LeftShiftAssignment& left_shift_assignment)
{}

void AstString::visit(const RightShiftAssignment& right_shift_assignment)
{}

void AstString::visit(Identifier identifier)
{
	add_line(std::format("identifier `{}`", identifier.name));
}
