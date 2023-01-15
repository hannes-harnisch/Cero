#pragma once

#include "SyntaxTree.hpp"

namespace cero
{

class AstComparator
{
	const SyntaxTree& left_ast;
	const SyntaxTree& right_ast;

public:
	AstComparator(const SyntaxTree& left, const SyntaxTree& right);

	bool compare() const;

private:
	template<typename T, size_t... I>
	bool nodes_equal(const T& left, const T& right, std::index_sequence<I...>) const;

	template<typename T>
	bool nodes_equal(const T& left, const T& right) const;

	template<typename T>
	requires std::is_aggregate_v<T>
	bool equal(const T& left, const T& right) const;

	template<typename T>
	static bool equal(T left, T right);

	template<typename T>
	bool equal(const std::vector<T>& left, const std::vector<T>& right) const;

	template<typename T>
	bool equal(std::span<T> left, std::span<T> right) const;

	bool equal(Definition left, Definition right) const;
	bool equal(Expression left, Expression right) const;
	bool equal(OptionalExpression left, OptionalExpression right) const;
};

} // namespace cero
