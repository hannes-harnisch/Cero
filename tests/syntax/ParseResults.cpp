#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

using namespace cero::ast;

TEST(ParseEmptyFunction)
{
	auto source = make_test_source(R"_____(
main()
{}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "main",
		.parameters = {},
		.outputs	= {},
		.statements = {},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

TEST(ParseSimpleFunction)
{
	auto source = make_test_source(R"_____(
fibonacci(var uint32 n) -> uint32
{
    var uint32 result = 0
    var uint32 next   = 1

    while n-- != 0
    {
        let temp = next
        next = result
        result += temp
    }

    return result
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "fibonacci",
		.parameters = {Function::Parameter {
			.specifier = ParameterSpecifier::Var,
			.name	   = "n",
			.type	   = ast.store(Identifier {"uint32"}),
		}},
		.outputs	= {FunctionOutput {
			   .type = ast.store(Identifier {"uint32"}),
		   }},
		.statements = {ast.store(Binding {
						   .specifier	= Binding::Specifier::Var,
						   .name		= "result",
						   .type		= ast.store(Identifier {"uint32"}),
						   .initializer = ast.store(NumericLiteral {
							   .kind = Literal::Decimal,
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Var,
						   .name		= "next",
						   .type		= ast.store(Identifier {"uint32"}),
						   .initializer = ast.store(NumericLiteral {
							   .kind = Literal::Decimal,
						   }),
					   }),
					   ast.store(WhileLoop {
						   .condition = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::Inequality,
							   .left  = ast.store(UnaryExpression {
									.op		 = UnaryOperator::PostDecrement,
									.operand = ast.store(Identifier {"n"}),
								}),
							   .right = ast.store(NumericLiteral {
								   .kind = Literal::Decimal,
							   }),
						   }),
						   .statement = ast.store(Block {
							   .statements = {ast.store(Binding {
												  .specifier   = Binding::Specifier::Let,
												  .name		   = "temp",
												  .initializer = ast.store(Identifier {"next"}),
											  }),
											  ast.store(BinaryExpression {
												  .op	 = BinaryOperator::Assign,
												  .left	 = ast.store(Identifier {"next"}),
												  .right = ast.store(Identifier {"result"}),
											  }),
											  ast.store(BinaryExpression {
												  .op	 = BinaryOperator::AddAssign,
												  .left	 = ast.store(Identifier {"result"}),
												  .right = ast.store(Identifier {"temp"}),
											  })},
						   }),
					   }),
					   ast.store(Return {
						   .expression = ast.store(Identifier {"result"}),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

TEST(ParseAdditiveAndMultiplicativeOperators)
{
	auto source = make_test_source(R"_____(
foo(int32 a, int32 b) -> int32
{
    let c = a + b
	let d = a + b * c
	let e = (d - a) / c
	return e ** 2 * b
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

TEST(ParseAdditiveAndComparisonOperators)
{
	auto source = make_test_source(R"_____(
bar(int32 a, int32 b, int32 c) -> bool
{
    let u = a - b == b + c
	let v = b * a != c / a
	let w = c + b > b * a
	let x = b / a < c - b
	let y = a * c <= b - a
	let z = b + c >= a / c
	return u || v || w || x || y || z
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
							   .op	  = BinaryOperator::Equality,
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
							   .op	  = BinaryOperator::Inequality,
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

TEST(ParseComparisonAndLogicalOperators)
{
	auto source = make_test_source(R"_____(
baz(int32 a, int32 b, int32 c, int32 d) -> bool
{
    return a + b == b + c && b + c != c + d && a < c && a > d
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
							   .op	  = BinaryOperator::Equality,
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
							  .op	 = BinaryOperator::Inequality,
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

TEST(ParseGenericReturnType)
{
	auto source = make_test_source(R"_____(
a() -> List<List<int32> >
{
	return ()
}

b() -> List<List<int32>>
{
	return ()
}

c() -> List<List<List<int32> > >
{
	return ()
}

d() -> List<List<List<int32> >>
{
	return ()
}

e() -> List<List<List<int32>> >
{
	return ()
}

f() -> List<List<List<int32>>>
{
	return ()
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "a",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {ast.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {ast.store(Identifier {"int32"})},
				   })},
			   }),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.arguments = {},
			}),
		})},
	}));

	ast.add_to_root(ast.store(Function {
		.name		= "b",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {ast.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {ast.store(Identifier {"int32"})},
				   })},
			   }),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.arguments = {},
			}),
		})},
	}));

	ast.add_to_root(ast.store(Function {
		.name		= "c",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {ast.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {ast.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {ast.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.arguments = {},
			}),
		})},
	}));

	ast.add_to_root(ast.store(Function {
		.name		= "d",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {ast.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {ast.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {ast.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.arguments = {},
			}),
		})},
	}));

	ast.add_to_root(ast.store(Function {
		.name		= "e",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {ast.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {ast.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {ast.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.arguments = {},
			}),
		})},
	}));

	ast.add_to_root(ast.store(Function {
		.name		= "f",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {ast.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {ast.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {ast.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.arguments = {},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}
