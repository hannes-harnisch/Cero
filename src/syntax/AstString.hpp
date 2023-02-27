#pragma once

#include "cero/driver/Source.hpp"
#include "cero/syntax/AstVisitor.hpp"
#include "cero/syntax/SyntaxTree.hpp"
#include "cero/util/LookupTable.hpp"

namespace cero
{

class AstString : public AstVisitor
{
	struct Edge
	{
		std::string_view branch;
		std::string_view prefix;
	};

	static constexpr Edge BODY {"├── ", "│   "};
	static constexpr Edge TAIL {"└── ", "    "};

	std::string				string;
	const SyntaxTree&		ast;
	const Source&			source;
	std::stack<std::string> prefixes;
	const Edge*				edge = &BODY;

public:
	AstString(const SyntaxTree& ast, const Source& source);

	std::string build();

private:
	void push_level();
	void pop_level();
	void set_tail(bool at_tail);
	void add_line(std::string_view text);
	void add_body_line(std::string_view text);
	void add_tail_line(std::string_view text);
	void visit(Definition definition);
	void visit(Expression expression);
	void visit_body(Expression expression);
	void visit_tail(Expression expression);
	void visit_optional(OptionalExpression optional_expression);
	void visit_each_in(const auto& list);
	void visit(const ast::Root& root) override;
	void visit(const ast::Function& function) override;
	void visit(const ast::Function::Parameter& parameter);
	void visit(const ast::FunctionOutput& output);
	void visit(const ast::Struct& struct_definition) override;
	void visit(const ast::Enum& enum_definition) override;
	void visit(const ast::Identifier& id) override;
	void visit(const ast::GenericIdentifier& generic_id) override;
	void visit(const ast::Variability& variability) override;
	void visit(const ast::ArrayType& array_type) override;
	void visit(const ast::PointerType& pointer_type) override;
	void visit(const ast::FunctionType& function_type) override;
	void visit(const ast::FunctionType::Parameter& parameter);
	void visit(const ast::NumericLiteral& numeric_literal) override;
	void visit(const ast::StringLiteral& string_literal) override;
	void visit(const ast::Binding& binding) override;
	void visit(const ast::Block& block) override;
	void visit(const ast::If& if_expression) override;
	void visit(const ast::WhileLoop& while_loop) override;
	void visit(const ast::ForLoop& for_loop) override;
	void visit(const ast::Break& break_expression) override;
	void visit(const ast::Continue& continue_expression) override;
	void visit(const ast::Return& return_expression) override;
	void visit(const ast::Throw& throw_expression) override;
	void visit(const ast::MemberAccess& member_access) override;
	void visit(const ast::Call& call) override;
	void visit(const ast::Index& index) override;
	void visit(const ast::UnaryExpression& unary_expression) override;
	void visit(const ast::BinaryExpression& binary_expression) override;
};

constexpr inline LookupTable<ast::UnaryOperator, std::string_view> UNARY_OPERATOR_STRINGS = []
{
	using enum ast::UnaryOperator;

	LookupTable<ast::UnaryOperator, std::string_view> t;
	t[PreIncrement]	 = "prefix ++";
	t[PreDecrement]	 = "prefix --";
	t[PostIncrement] = "postfix ++";
	t[PostDecrement] = "postfix --";
	t[AddressOf]	 = "&";
	t[Dereference]	 = "^";
	t[Negate]		 = "-";
	t[LogicalNot]	 = "!";
	t[BitwiseNot]	 = "~";
	return t;
}();

constexpr inline LookupTable<ast::BinaryOperator, std::string_view> BINARY_OPERATOR_STRINGS = []
{
	using enum ast::BinaryOperator;

	LookupTable<ast::BinaryOperator, std::string_view> t;
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
	t[Equal]			= "==";
	t[NotEqual]			= "!=";
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
	t[AndAssign]		= "&=";
	t[OrAssign]			= "|=";
	t[XorAssign]		= "~=";
	t[LeftShiftAssign]	= "<<=";
	t[RightShiftAssign] = ">>=";
	return t;
}();

} // namespace cero
