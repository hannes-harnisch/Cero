#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

using namespace cero::ast;

CERO_TEST(ParseAdditiveAndMultiplicativeOperators)
{
	auto source = make_test_source(R"_____(
foo(int32 a, int32 b) -> int32
{
    let c = a + b;
	let d = a + b * c;
	let e = (d - a) / c;
	return e ** 2 * b;
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "foo",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "a",
						   .type	  = ast.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "b",
						   .type	  = ast.store(Identifier {"int32"}),
					   }},
		.outputs	= {FunctionOutput {
			   .type = ast.store(Identifier {"int32"}),
		   }},
		.statements = {ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "c",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Add,
							   .left  = ast.store(Identifier {"a"}),
							   .right = ast.store(Identifier {"b"}),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "d",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Add,
							   .left  = ast.store(Identifier {"a"}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Multiply,
								   .left  = ast.store(Identifier {"b"}),
								   .right = ast.store(Identifier {"c"}),
							   }),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "e",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Divide,
							   .left  = ast.store(Call {
									.arguments = {ast.store(BinaryExpression {
										.op	   = BinaryOperator::Subtract,
										.left  = ast.store(Identifier {"d"}),
										.right = ast.store(Identifier {"a"}),
									})},
								}),
							   .right = ast.store(Identifier {"c"}),
						   }),
					   }),
					   ast.store(Return {
						   .expression = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Multiply,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Power,
									.left  = ast.store(Identifier {"e"}),
									.right = ast.store(NumericLiteral {
										.kind = Literal::Decimal,
									}),
								}),
							   .right = ast.store(Identifier {"b"}),
						   }),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

CERO_TEST(ParseAdditiveAndComparisonOperators)
{
	auto source = make_test_source(R"_____(
bar(int32 a, int32 b, int32 c) -> bool
{
    let u = a - b == b + c;
	let v = b * a != c / a;
	let w = c + b > b * a;
	let x = b / a < c - b;
	let y = a * c <= b - a;
	let z = b + c >= a / c;
	return u || v || w || x || y || z;
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "bar",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "a",
						   .type	  = ast.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "b",
						   .type	  = ast.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "c",
						   .type	  = ast.store(Identifier {"int32"}),
					   }},
		.outputs	= {FunctionOutput {
			   .type = ast.store(Identifier {"bool"}),
		   }},
		.statements = {ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "u",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Equal,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Subtract,
									.left  = ast.store(Identifier {"a"}),
									.right = ast.store(Identifier {"b"}),
								}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Add,
								   .left  = ast.store(Identifier {"b"}),
								   .right = ast.store(Identifier {"c"}),
							   }),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "v",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::NotEqual,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Multiply,
									.left  = ast.store(Identifier {"b"}),
									.right = ast.store(Identifier {"a"}),
								}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Divide,
								   .left  = ast.store(Identifier {"c"}),
								   .right = ast.store(Identifier {"a"}),
							   }),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "w",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Greater,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Add,
									.left  = ast.store(Identifier {"c"}),
									.right = ast.store(Identifier {"b"}),
								}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Multiply,
								   .left  = ast.store(Identifier {"b"}),
								   .right = ast.store(Identifier {"a"}),
							   }),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "x",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Less,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Divide,
									.left  = ast.store(Identifier {"b"}),
									.right = ast.store(Identifier {"a"}),
								}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Subtract,
								   .left  = ast.store(Identifier {"c"}),
								   .right = ast.store(Identifier {"b"}),
							   }),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "y",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::LessEqual,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Multiply,
									.left  = ast.store(Identifier {"a"}),
									.right = ast.store(Identifier {"c"}),
								}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Subtract,
								   .left  = ast.store(Identifier {"b"}),
								   .right = ast.store(Identifier {"a"}),
							   }),
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "z",
						   .initializer = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::GreaterEqual,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Add,
									.left  = ast.store(Identifier {"b"}),
									.right = ast.store(Identifier {"c"}),
								}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Divide,
								   .left  = ast.store(Identifier {"a"}),
								   .right = ast.store(Identifier {"c"}),
							   }),
						   }),
					   }),
					   ast.store(Return {
						   .expression = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::LogicalOr,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::LogicalOr,
									.left  = ast.store(BinaryExpression {
										 .op	= BinaryOperator::LogicalOr,
										 .left	= ast.store(BinaryExpression {
											  .op	 = BinaryOperator::LogicalOr,
											  .left	 = ast.store(BinaryExpression {
												   .op	  = BinaryOperator::LogicalOr,
												   .left  = ast.store(Identifier {"u"}),
												   .right = ast.store(Identifier {"v"}),
										   }),
											  .right = ast.store(Identifier {"w"}),
										  }),
										 .right = ast.store(Identifier {"x"}),
									 }),
									.right = ast.store(Identifier {"y"}),
								}),
							   .right = ast.store(Identifier {"z"}),
						   }),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

CERO_TEST(ParseComparisonAndLogicalOperators)
{
	auto source = make_test_source(R"_____(
baz(int32 a, int32 b, int32 c, int32 d) -> bool
{
    return a + b == b + c && b + c != c + d && a < c && a > d;
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "baz",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "a",
						   .type	  = ast.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "b",
						   .type	  = ast.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "c",
						   .type	  = ast.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "d",
						   .type	  = ast.store(Identifier {"int32"}),
					   }},
		.outputs	= {FunctionOutput {
			   .type = ast.store(Identifier {"bool"}),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(BinaryExpression {
				.op	   = BinaryOperator::LogicalAnd,
				.left  = ast.store(BinaryExpression {
					 .op	= BinaryOperator::LogicalAnd,
					 .left	= ast.store(BinaryExpression {
						  .op	 = BinaryOperator::LogicalAnd,
						  .left	 = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Equal,
							   .left  = ast.store(BinaryExpression {
									.op	   = BinaryOperator::Add,
									.left  = ast.store(Identifier {"a"}),
									.right = ast.store(Identifier {"b"}),
							}),
							   .right = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Add,
								   .left  = ast.store(Identifier {"b"}),
								   .right = ast.store(Identifier {"c"}),
							   }),
						   }),
						  .right = ast.store(BinaryExpression {
							  .op	 = BinaryOperator::NotEqual,
							  .left	 = ast.store(BinaryExpression {
								   .op	  = BinaryOperator::Add,
								   .left  = ast.store(Identifier {"b"}),
								   .right = ast.store(Identifier {"c"}),
							   }),
							  .right = ast.store(BinaryExpression {
								  .op	 = BinaryOperator::Add,
								  .left	 = ast.store(Identifier {"c"}),
								  .right = ast.store(Identifier {"d"}),
							  }),
						  }),
					  }),
					 .right = ast.store(BinaryExpression {
						 .op	= BinaryOperator::Less,
						 .left	= ast.store(Identifier {"a"}),
						 .right = ast.store(Identifier {"c"}),
					 }),
				 }),
				.right = ast.store(BinaryExpression {
					.op	   = BinaryOperator::Greater,
					.left  = ast.store(Identifier {"a"}),
					.right = ast.store(Identifier {"d"}),
				}),
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}
