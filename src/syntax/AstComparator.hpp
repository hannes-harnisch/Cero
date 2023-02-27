#pragma once

#include "cero/syntax/AstVisitor.hpp"
#include "cero/syntax/SyntaxTree.hpp"

namespace cero
{

class AstComparator : public AstVisitor
{
	const SyntaxTree&	  left_ast;
	const SyntaxTree&	  right_ast;
	const void*			  left_node = nullptr;
	const std::type_info* left_type = nullptr;
	Definition			  right_def;
	Expression			  right_expr;
	bool				  ready = false;
	bool				  equal = true;

public:
	AstComparator(const SyntaxTree& left, const SyntaxTree& right);

	bool compare();

private:
	template<bool IS_EXPRESSION, typename N>
	void dispatch_visit(const N& node);

	void visit(const ast::Root& root) override;
	void visit(const ast::Function& function) override;
	void visit(const ast::Struct& struct_definition) override;
	void visit(const ast::Enum& enum_definition) override;
	void visit(const ast::Identifier& id) override;
	void visit(const ast::GenericIdentifier& generic_id) override;
	void visit(const ast::Variability& variability) override;
	void visit(const ast::ArrayType& array_type) override;
	void visit(const ast::PointerType& pointer_type) override;
	void visit(const ast::FunctionType& function_type) override;
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

	template<typename T>
	bool equate_nodes(const T& left, const T& right);

	template<typename T, size_t... I>
	bool equate_node_fields(const T& left, const T& right, std::index_sequence<I...>);

	template<typename T>
	static bool equate(T left, T right);

	template<typename T>
	bool equate(const T& left, const T& right)
	requires std::is_aggregate_v<T>;

	template<typename T>
	bool equate(const std::vector<T>& left, const std::vector<T>& right);

	template<typename T>
	bool equate(std::span<T> left, std::span<T> right);

	bool equate(Definition left, Definition right);
	bool equate(Expression left, Expression right);
	bool equate(OptionalExpression left, OptionalExpression right);
};

} // namespace cero
