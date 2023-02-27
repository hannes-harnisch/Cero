#include "AstComparator.hpp"

#include <boost/pfr.hpp>

namespace cero
{

AstComparator::AstComparator(const SyntaxTree& left, const SyntaxTree& right) :
	left_ast(left),
	right_ast(right),
	right_def(~0u),
	right_expr(~0u)
{}

bool AstComparator::compare()
{
	left_ast.visit(*this);
	return equal;
}

template<bool IS_EXPRESSION, typename N>
void AstComparator::dispatch_visit(const N& node)
{
	if (ready)
	{
		ready = false;
		if (*left_type == typeid(N))
			equal = equal && equate_nodes(*static_cast<const N*>(left_node), node);
		else
			equal = false;
	}
	else
	{
		left_node = &node;
		left_type = &typeid(N);
		ready	  = true;
		if constexpr (IS_EXPRESSION)
			right_ast.visit(*this, right_expr);
		else
			right_ast.visit(*this, right_def);
	}
}

void AstComparator::visit(const ast::Root& root)
{
	if (ready)
	{
		ready = false;
		equal = equal && equate_nodes(*static_cast<const ast::Root*>(left_node), root);
	}
	else
	{
		left_node = &root;
		ready	  = true;
		right_ast.visit(*this);
	}
}

void AstComparator::visit(const ast::Function& function)
{
	dispatch_visit<false>(function);
}

void AstComparator::visit(const ast::Struct& struct_definition)
{
	dispatch_visit<false>(struct_definition);
}

void AstComparator::visit(const ast::Enum& enum_definition)
{
	dispatch_visit<false>(enum_definition);
}

void AstComparator::visit(const ast::Identifier& id)
{
	dispatch_visit<true>(id);
}

void AstComparator::visit(const ast::GenericIdentifier& generic_id)
{
	dispatch_visit<true>(generic_id);
}

void AstComparator::visit(const ast::Variability& variability)
{
	dispatch_visit<true>(variability);
}

void AstComparator::visit(const ast::ArrayType& array_type)
{
	dispatch_visit<true>(array_type);
}

void AstComparator::visit(const ast::PointerType& pointer_type)
{
	dispatch_visit<true>(pointer_type);
}

void AstComparator::visit(const ast::FunctionType& function_type)
{
	dispatch_visit<true>(function_type);
}

void AstComparator::visit(const ast::NumericLiteral& numeric_literal)
{
	dispatch_visit<true>(numeric_literal);
}

void AstComparator::visit(const ast::StringLiteral& string_literal)
{
	dispatch_visit<true>(string_literal);
}

void AstComparator::visit(const ast::Binding& binding)
{
	dispatch_visit<true>(binding);
}

void AstComparator::visit(const ast::Block& block)
{
	dispatch_visit<true>(block);
}

void AstComparator::visit(const ast::If& if_expression)
{
	dispatch_visit<true>(if_expression);
}

void AstComparator::visit(const ast::WhileLoop& while_loop)
{
	dispatch_visit<true>(while_loop);
}

void AstComparator::visit(const ast::ForLoop& for_loop)
{
	dispatch_visit<true>(for_loop);
}

void AstComparator::visit(const ast::Break& break_expression)
{
	dispatch_visit<true>(break_expression);
}

void AstComparator::visit(const ast::Continue& continue_expression)
{
	dispatch_visit<true>(continue_expression);
}

void AstComparator::visit(const ast::Return& return_expression)
{
	dispatch_visit<true>(return_expression);
}

void AstComparator::visit(const ast::Throw& throw_expression)
{
	dispatch_visit<true>(throw_expression);
}

void AstComparator::visit(const ast::MemberAccess& member_access)
{
	dispatch_visit<true>(member_access);
}

void AstComparator::visit(const ast::Call& call)
{
	dispatch_visit<true>(call);
}

void AstComparator::visit(const ast::Index& index)
{
	dispatch_visit<true>(index);
}

void AstComparator::visit(const ast::UnaryExpression& unary_expression)
{
	dispatch_visit<true>(unary_expression);
}

void AstComparator::visit(const ast::BinaryExpression& binary_expression)
{
	dispatch_visit<true>(binary_expression);
}

template<typename T>
bool AstComparator::equate_nodes(const T& left, const T& right)
{
	return equate_node_fields(left, right, std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
}

template<typename T, size_t... I>
bool AstComparator::equate_node_fields(const T& left, const T& right, std::index_sequence<I...>)
{
	return (... && equate(boost::pfr::get<I>(left), boost::pfr::get<I>(right)));
}

template<typename T>
bool AstComparator::equate(T left, T right)
{
	return left == right;
}

template<typename T>
bool AstComparator::equate(const T& left, const T& right)
requires std::is_aggregate_v<T>
{
	return equate_nodes(left, right);
}

template<typename T>
bool AstComparator::equate(const std::vector<T>& left, const std::vector<T>& right)
{
	return equate(std::span(left), std::span(right));
}

template<typename T>
bool AstComparator::equate(std::span<T> left, std::span<T> right)
{
	size_t size = left.size();
	if (size != right.size())
		return false;

	for (size_t i = 0; i != size; ++i)
		if (!equate(left[i], right[i]))
			return false;

	return true;
}

bool AstComparator::equate(Definition left, Definition right)
{
	right_def = right;
	left_ast.visit(*this, left);
	return equal;
}

bool AstComparator::equate(Expression left, Expression right)
{
	right_expr = right;
	left_ast.visit(*this, left);
	return equal;
}

bool AstComparator::equate(OptionalExpression left, OptionalExpression right)
{
	bool left_null	= left.is_null();
	bool right_null = right.is_null();
	if (!left_null && !right_null)
		return equate(left.get(), right.get());

	return left_null == right_null;
}

} // namespace cero
