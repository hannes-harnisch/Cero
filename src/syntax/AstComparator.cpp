#include "AstComparator.hpp"

#include <boost/pfr.hpp>

namespace cero
{

AstComparator::AstComparator(const SyntaxTree& left, const SyntaxTree& right) :
	left_ast(left),
	right_ast(right)
{}

bool AstComparator::compare() const
{
	return equal(left_ast.get_root(), right_ast.get_root());
}

template<typename T, size_t... I>
bool AstComparator::nodes_equal(const T& left, const T& right, std::index_sequence<I...>) const
{
	return (... && equal(boost::pfr::get<I>(left), boost::pfr::get<I>(right)));
}

template<typename T>
bool AstComparator::nodes_equal(const T& left, const T& right) const
{
	return nodes_equal(left, right, std::make_index_sequence<boost::pfr::tuple_size_v<T>> {});
}

template<typename T>
requires std::is_aggregate_v<T>
bool AstComparator::equal(const T& left, const T& right) const
{
	return nodes_equal(left, right);
}

template<typename T>
bool AstComparator::equal(T left, T right)
{
	return left == right;
}

template<typename T>
bool AstComparator::equal(const std::vector<T>& left, const std::vector<T>& right) const
{
	return equal(std::span(left), std::span(right));
}

template<typename T>
bool AstComparator::equal(std::span<T> left, std::span<T> right) const
{
	size_t size = left.size();
	if (size != right.size())
		return false;

	for (size_t i = 0; i != size; ++i)
		if (!equal(left[i], right[i]))
			return false;

	return true;
}

bool AstComparator::equal(Definition left, Definition right) const
{
	auto& left_def	= left_ast.get(left);
	auto& right_def = right_ast.get(right);

	auto visitor = [&]<typename T>(const T& left_node) -> bool
	{
		const T* right_node = std::get_if<T>(&right_def);
		if (right_node == nullptr)
			return false;

		return nodes_equal(left_node, *right_node);
	};
	return std::visit(visitor, left_def);
}

bool AstComparator::equal(Expression left, Expression right) const
{
	auto& left_expr	 = left_ast.get(left);
	auto& right_expr = right_ast.get(right);

	auto visitor = [&]<typename T>(const T& left_node) -> bool
	{
		const T* right_node = std::get_if<T>(&right_expr);
		if (right_node == nullptr)
			return false;

		return nodes_equal(left_node, *right_node);
	};
	return std::visit(visitor, left_expr);
}

bool AstComparator::equal(OptionalExpression left, OptionalExpression right) const
{
	bool left_null	= left.is_null();
	bool right_null = right.is_null();
	if (!left_null && !right_null)
		return equal(left.get(), right.get());

	return left_null == right_null;
}

} // namespace cero