#pragma once

#include "AstState.hpp"
#include "driver/Source.hpp"
#include "syntax/Definition.hpp"
#include "syntax/Expression.hpp"

namespace cero
{

class SyntaxTree
{
	AstState state;

public:
	explicit SyntaxTree(AstState state);

	std::span<const Definition> get_root() const;
	const ExpressionNode&		get(Expression expression) const;
	const DefinitionNode&		get(Definition definition) const;

	bool operator==(const SyntaxTree& other) const;

	std::string to_string(const Source& source) const;
	void		log(const Source& source) const;
};

} // namespace cero