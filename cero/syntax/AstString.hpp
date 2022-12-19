#pragma once

#include "driver/Source.hpp"
#include "syntax/SyntaxTree.hpp"

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
	void visit_node(const Function& function);
	void visit(const Parameter& parameter);
	void visit(const ReturnValue& return_value);
	void visit_node(const Struct& struct_definition);
	void visit_node(const Enum& enum_definition);
	void visit(Expression expression);
	void visit_body(Expression expression);
	void visit_tail(Expression expression);
	void visit_optional(OptionalExpression optional_expression);
	void visit_node(Identifier id);
	void visit_node(const GenericIdentifier& generic_id);
	void visit_node(const Variability& variability);
	void visit_node(const ArrayTypeExpression& array_type);
	void visit_node(const PointerTypeExpression& pointer_type);
	void visit_node(const NumericLiteral& numeric_literal);
	void visit_node(const StringLiteral& string_literal);
	void visit_node(const Binding& binding);
	void visit_node(const BlockExpression& block);
	void visit_node(const GroupExpression& group);
	void visit_node(const IfExpression& if_expression);
	void visit_node(const WhileLoop& while_loop);
	void visit_node(const ForLoop& for_loop);
	void visit_node(const BreakExpression& break_expression);
	void visit_node(const ContinueExpression& continue_expression);
	void visit_node(const ReturnExpression& return_expression);
	void visit_node(const ThrowExpression& throw_expression);
	void visit_node(const MemberAccess& member_access);
	void visit_node(const CallExpression& call);
	void visit_node(const IndexExpression& index);
	void visit_node(const UnaryExpression& unary_expression);
	void visit_node(const BinaryExpression& binary_expression);
};
