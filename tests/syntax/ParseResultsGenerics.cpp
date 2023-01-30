#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

using namespace cero::ast;

CERO_TEST(ParseGenericReturnType)
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

CERO_TEST(ParseLessAndRightShift)
{
	auto source = make_test_source(R"_____(
oof(int32 a, int32 b) -> bool
{
	return a < b >> 16
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "oof",
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
			   .type = ast.store(Identifier {"bool"}),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(BinaryExpression {
				.op	   = BinaryOperator::Less,
				.left  = ast.store(Identifier {"a"}),
				.right = ast.store(BinaryExpression {
					.op	   = BinaryOperator::RightShift,
					.left  = ast.store(Identifier {"b"}),
					.right = ast.store(NumericLiteral {
						.kind = Literal::Decimal,
					}),
				}),
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

CERO_TEST(ParseAmbiguousGenericCallVsComparisonArguments)
{
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64
{
	return a(b<c,d>(e))
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "ouch",
		.parameters = {Function::Parameter {
			.specifier = ParameterSpecifier::In,
			.name	   = "e",
			.type	   = ast.store(Identifier {"float32"}),
		}},
		.outputs	= {FunctionOutput {
			   .type = ast.store(Identifier {"float64"}),
		   }},
		.statements = {ast.store(Return {
			.expression = ast.store(Call {
				.callee	   = ast.store(Identifier {"a"}),
				.arguments = {ast.store(Call {
					.callee	   = ast.store(GenericIdentifier {
						   .name	  = "b",
						   .arguments = {ast.store(Identifier {"c"}), ast.store(Identifier {"d"})},
					   }),
					.arguments = {ast.store(Identifier {"e"})},
				})},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}
