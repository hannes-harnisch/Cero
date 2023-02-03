#pragma once

#include "cero/syntax/Definition.hpp"
#include "cero/syntax/Expression.hpp"
#include "cero/syntax/Literal.hpp"

namespace cero
{

class AstVisitor
{
public:
	virtual ~AstVisitor() = default;
};

} // namespace cero
