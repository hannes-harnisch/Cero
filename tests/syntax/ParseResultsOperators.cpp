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
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "foo",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "a",
						   .type	  = b.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "b",
						   .type	  = b.store(Identifier {"int32"}),
					   }},
		.outputs	= {FunctionOutput {
			   .type = b.store(Identifier {"int32"}),
		   }},
		.statements = {b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "c",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Add,
							   .left  = b.store(Identifier {"a"}),
							   .right = b.store(Identifier {"b"}),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "d",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Add,
							   .left  = b.store(Identifier {"a"}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Multiply,
								   .left  = b.store(Identifier {"b"}),
								   .right = b.store(Identifier {"c"}),
							   }),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "e",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Divide,
							   .left  = b.store(Call {
									.arguments = {b.store(BinaryExpression {
										.op	   = BinaryOperator::Subtract,
										.left  = b.store(Identifier {"d"}),
										.right = b.store(Identifier {"a"}),
									})},
								}),
							   .right = b.store(Identifier {"c"}),
						   }),
					   }),
					   b.store(Return {
						   .expression = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Multiply,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Power,
									.left  = b.store(Identifier {"e"}),
									.right = b.store(NumericLiteral {
										.kind = Literal::Decimal,
									}),
								}),
							   .right = b.store(Identifier {"b"}),
						   }),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
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
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "bar",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "a",
						   .type	  = b.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "b",
						   .type	  = b.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "c",
						   .type	  = b.store(Identifier {"int32"}),
					   }},
		.outputs	= {FunctionOutput {
			   .type = b.store(Identifier {"bool"}),
		   }},
		.statements = {b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "u",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Equal,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Subtract,
									.left  = b.store(Identifier {"a"}),
									.right = b.store(Identifier {"b"}),
								}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Add,
								   .left  = b.store(Identifier {"b"}),
								   .right = b.store(Identifier {"c"}),
							   }),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "v",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::NotEqual,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Multiply,
									.left  = b.store(Identifier {"b"}),
									.right = b.store(Identifier {"a"}),
								}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Divide,
								   .left  = b.store(Identifier {"c"}),
								   .right = b.store(Identifier {"a"}),
							   }),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "w",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Greater,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Add,
									.left  = b.store(Identifier {"c"}),
									.right = b.store(Identifier {"b"}),
								}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Multiply,
								   .left  = b.store(Identifier {"b"}),
								   .right = b.store(Identifier {"a"}),
							   }),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "x",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Less,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Divide,
									.left  = b.store(Identifier {"b"}),
									.right = b.store(Identifier {"a"}),
								}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Subtract,
								   .left  = b.store(Identifier {"c"}),
								   .right = b.store(Identifier {"b"}),
							   }),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "y",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::LessEqual,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Multiply,
									.left  = b.store(Identifier {"a"}),
									.right = b.store(Identifier {"c"}),
								}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Subtract,
								   .left  = b.store(Identifier {"b"}),
								   .right = b.store(Identifier {"a"}),
							   }),
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "z",
						   .initializer = b.store(BinaryExpression {
							   .op	  = BinaryOperator::GreaterEqual,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Add,
									.left  = b.store(Identifier {"b"}),
									.right = b.store(Identifier {"c"}),
								}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Divide,
								   .left  = b.store(Identifier {"a"}),
								   .right = b.store(Identifier {"c"}),
							   }),
						   }),
					   }),
					   b.store(Return {
						   .expression = b.store(BinaryExpression {
							   .op	  = BinaryOperator::LogicalOr,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::LogicalOr,
									.left  = b.store(BinaryExpression {
										 .op	= BinaryOperator::LogicalOr,
										 .left	= b.store(BinaryExpression {
											  .op	 = BinaryOperator::LogicalOr,
											  .left	 = b.store(BinaryExpression {
												   .op	  = BinaryOperator::LogicalOr,
												   .left  = b.store(Identifier {"u"}),
												   .right = b.store(Identifier {"v"}),
										   }),
											  .right = b.store(Identifier {"w"}),
										  }),
										 .right = b.store(Identifier {"x"}),
									 }),
									.right = b.store(Identifier {"y"}),
								}),
							   .right = b.store(Identifier {"z"}),
						   }),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
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
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "baz",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "a",
						   .type	  = b.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "b",
						   .type	  = b.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "c",
						   .type	  = b.store(Identifier {"int32"}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "d",
						   .type	  = b.store(Identifier {"int32"}),
					   }},
		.outputs	= {FunctionOutput {
			   .type = b.store(Identifier {"bool"}),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(BinaryExpression {
				.op	   = BinaryOperator::LogicalAnd,
				.left  = b.store(BinaryExpression {
					 .op	= BinaryOperator::LogicalAnd,
					 .left	= b.store(BinaryExpression {
						  .op	 = BinaryOperator::LogicalAnd,
						  .left	 = b.store(BinaryExpression {
							   .op	  = BinaryOperator::Equal,
							   .left  = b.store(BinaryExpression {
									.op	   = BinaryOperator::Add,
									.left  = b.store(Identifier {"a"}),
									.right = b.store(Identifier {"b"}),
							}),
							   .right = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Add,
								   .left  = b.store(Identifier {"b"}),
								   .right = b.store(Identifier {"c"}),
							   }),
						   }),
						  .right = b.store(BinaryExpression {
							  .op	 = BinaryOperator::NotEqual,
							  .left	 = b.store(BinaryExpression {
								   .op	  = BinaryOperator::Add,
								   .left  = b.store(Identifier {"b"}),
								   .right = b.store(Identifier {"c"}),
							   }),
							  .right = b.store(BinaryExpression {
								  .op	 = BinaryOperator::Add,
								  .left	 = b.store(Identifier {"c"}),
								  .right = b.store(Identifier {"d"}),
							  }),
						  }),
					  }),
					 .right = b.store(BinaryExpression {
						 .op	= BinaryOperator::Less,
						 .left	= b.store(Identifier {"a"}),
						 .right = b.store(Identifier {"c"}),
					 }),
				 }),
				.right = b.store(BinaryExpression {
					.op	   = BinaryOperator::Greater,
					.left  = b.store(Identifier {"a"}),
					.right = b.store(Identifier {"d"}),
				}),
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}
