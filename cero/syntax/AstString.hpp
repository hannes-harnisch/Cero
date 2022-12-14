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
	void set_edge(const auto& container, size_t index);
	void add_line(std::string text);
	void visit(Definition definition);
	void visit(const Function& function);
	void visit(const Parameter& parameter);
	void visit(const ReturnValue& return_value);
	void visit(const Struct& struct_node);
	void visit(const Enum& enum_node);
	void visit(Expression expression);
	void visit(const BinaryExpression& binary_expression);
	void visit(const GenericIdentifier&);
	void visit(const ArrayTypeExpression&);
	void visit(const PointerTypeExpression&);
	void visit(const NumericLiteral&);
	void visit(const StringLiteral&);
	void visit(const LetBinding&);
	void visit(const VarBinding&);
	void visit(const BlockExpression&);
	void visit(const GroupExpression&);
	void visit(const IfExpression&);
	void visit(const WhileLoop&);
	void visit(const ForLoop&);
	void visit(const Access& access);
	void visit(const Call& call);
	void visit(const Indexing& indexing);
	void visit(const BreakExpression& break_expression);
	void visit(const ContinueExpression& continue_expression);
	void visit(const ReturnExpression& return_expression);
	void visit(const ThrowExpression& throw_expression);
	void visit(const TryExpression& try_expression);
	void visit(const PreIncrement& pre_increment);
	void visit(const PreDecrement& pre_decrement);
	void visit(const PostIncrement& post_increment);
	void visit(const PostDecrement& post_decrement);
	void visit(const AddressOf& address_of);
	void visit(const Dereference& dereference);
	void visit(const Negation& negation);
	void visit(const LogicalNot& logical_not);
	void visit(const BitwiseNot& bitwise_not);
	void visit(const Addition& addition);
	void visit(const Subtraction& subtraction);
	void visit(const Multiplication& multiplication);
	void visit(const Division& division);
	void visit(const Remainder& remainder);
	void visit(const Exponentiation& exponentiation);
	void visit(const LogicalAnd& logical_and);
	void visit(const LogicalOr& logical_or);
	void visit(const BitwiseAnd& bitwise_and);
	void visit(const BitwiseOr& bitwise_or);
	void visit(const Xor& xor_expression);
	void visit(const LeftShift& left_shift);
	void visit(const RightShift& right_shift);
	void visit(const Equality& equality);
	void visit(const Inequality& inequality);
	void visit(const Less& less);
	void visit(const Greater& greater);
	void visit(const LessEqual& less_equal);
	void visit(const GreaterEqual& greater_equal);
	void visit(const Assignment& assignment);
	void visit(const AdditionAssignment& addition_assignment);
	void visit(const SubtractionAssignment& subtraction_assignment);
	void visit(const MultiplicationAssignment& multiplication_assignment);
	void visit(const DivisionAssignment& division_assignment);
	void visit(const RemainderAssignment& remainder_assignment);
	void visit(const ExponentiationAssignment& exponentiation_assignment);
	void visit(const BitwiseAndAssignment& bitwise_and_assignment);
	void visit(const BitwiseOrAssignment& bitwise_or_assignment);
	void visit(const XorAssignment& xor_assignment);
	void visit(const LeftShiftAssignment& left_shift_assignment);
	void visit(const RightShiftAssignment& right_shift_assignment);
	void visit(Identifier identifier);
};
