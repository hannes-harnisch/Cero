#pragma once

#include "driver/Source.hpp"
#include "syntax/SyntaxTree.hpp"

namespace cero
{

class AstString
{
	struct Edge
	{
		std::string_view branch;
		std::string_view prefix;
	};

	static constexpr Edge BODY {"├── ", "│   "};
	static constexpr Edge TAIL {"└── ", "    "};

	using StringStack = std::stack<std::string, std::vector<std::string>>;

	std::string		  string;
	const SyntaxTree& ast;
	const Source&	  source;
	StringStack		  prefixes;
	const Edge*		  edge = &BODY;

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
	void visit_each_in(const auto& list);
	void visit(Definition definition);
	void visit_node(const ast::Function& function);
	void visit(const ast::Parameter& parameter);
	void visit(const ast::ReturnValue& return_value);
	void visit_node(const ast::Struct& struct_definition);
	void visit_node(const ast::Enum& enum_definition);
	void visit(Expression expression);
	void visit_body(Expression expression);
	void visit_tail(Expression expression);
	void visit_optional(OptionalExpression optional_expression);
	void visit_node(ast::Identifier id);
	void visit_node(const ast::GenericIdentifier& generic_id);
	void visit_node(const ast::Variability& variability);
	void visit_node(const ast::ArrayType& array_type);
	void visit_node(const ast::PointerType& pointer_type);
	void visit_node(const ast::NumericLiteral& numeric_literal);
	void visit_node(const ast::StringLiteral& string_literal);
	void visit_node(const ast::Binding& binding);
	void visit_node(const ast::Block& block);
	void visit_node(const ast::If& if_expression);
	void visit_node(const ast::WhileLoop& while_loop);
	void visit_node(const ast::ForLoop& for_loop);
	void visit_node(const ast::Break& break_expression);
	void visit_node(const ast::Continue& continue_expression);
	void visit_node(const ast::Return& return_expression);
	void visit_node(const ast::Throw& throw_expression);
	void visit_node(const ast::MemberAccess& member_access);
	void visit_node(const ast::Call& call);
	void visit_node(const ast::Index& index);
	void visit_node(const ast::UnaryExpression& unary_expression);
	void visit_node(const ast::BinaryExpression& binary_expression);
};

} // namespace cero