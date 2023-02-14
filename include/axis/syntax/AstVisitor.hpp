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

	virtual void visit(const ast::Function&)		  = 0;
	virtual void visit(const ast::Struct&)			  = 0;
	virtual void visit(const ast::Enum&)			  = 0;
	virtual void visit(const ast::Identifier&)		  = 0;
	virtual void visit(const ast::GenericIdentifier&) = 0;
	virtual void visit(const ast::Variability&)		  = 0;
	virtual void visit(const ast::ArrayType&)		  = 0;
	virtual void visit(const ast::PointerType&)		  = 0;
	virtual void visit(const ast::FunctionType&)	  = 0;
	virtual void visit(const ast::NumericLiteral&)	  = 0;
	virtual void visit(const ast::StringLiteral&)	  = 0;
	virtual void visit(const ast::Binding&)			  = 0;
	virtual void visit(const ast::Block&)			  = 0;
	virtual void visit(const ast::If&)				  = 0;
	virtual void visit(const ast::WhileLoop&)		  = 0;
	virtual void visit(const ast::ForLoop&)			  = 0;
	virtual void visit(const ast::Break&)			  = 0;
	virtual void visit(const ast::Continue&)		  = 0;
	virtual void visit(const ast::Return&)			  = 0;
	virtual void visit(const ast::Throw&)			  = 0;
	virtual void visit(const ast::MemberAccess&)	  = 0;
	virtual void visit(const ast::Call&)			  = 0;
	virtual void visit(const ast::Index&)			  = 0;
	virtual void visit(const ast::UnaryExpression&)	  = 0;
	virtual void visit(const ast::BinaryExpression&)  = 0;
};

} // namespace cero
