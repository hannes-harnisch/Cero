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
	return a < b >> (16 - 4)
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
					.right = ast.store(Call {
						.arguments = {ast.store(BinaryExpression {
							.op	   = BinaryOperator::Subtract,
							.left  = ast.store(NumericLiteral {
								 .kind = Literal::Decimal,
							 }),
							.right = ast.store(NumericLiteral {
								.kind = Literal::Decimal,
							}),
						})},
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
	return a(b<c, d>(e))
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

CERO_TEST(ParseComparisonArgumentsVsGenericPattern)
{
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64
{
	return a(b < c, d > e)
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
				.arguments = {ast.store(BinaryExpression {
								  .op	 = BinaryOperator::Less,
								  .left	 = ast.store(Identifier {"b"}),
								  .right = ast.store(Identifier {"c"}),
							  }),
							  ast.store(BinaryExpression {
								  .op	 = BinaryOperator::Greater,
								  .left	 = ast.store(Identifier {"d"}),
								  .right = ast.store(Identifier {"e"}),
							  })},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

CERO_TEST(ParseComparisonAndRightShiftAsGenericArgument)
{
	auto source = make_test_source(R"_____(
woof() -> A<(B > C)>
{
	return ()
}

meow() -> A<(D >> E)>
{
	return ()
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "woof",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "A",
				   .arguments = {ast.store(Call {
					   .arguments = {ast.store(BinaryExpression {
						   .op	  = BinaryOperator::Greater,
						   .left  = ast.store(Identifier {"B"}),
						   .right = ast.store(Identifier {"C"}),
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
		.name		= "meow",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = ast.store(GenericIdentifier {
				   .name	  = "A",
				   .arguments = {ast.store(Call {
					   .arguments = {ast.store(BinaryExpression {
						   .op	  = BinaryOperator::RightShift,
						   .left  = ast.store(Identifier {"D"}),
						   .right = ast.store(Identifier {"E"}),
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

CERO_TEST(ParseGenericParameters)
{
	auto source = make_test_source(R"_____(
moo(List<int32> _a,
	List<List<int32>> _b,
	List<List<int32> > _c,
	List<List<List<int32> > > _d,
	List<List<List<int32> >> _e,
	List<List<List<int32>> > _f,
	List<List<List<int32>>> _g)
{}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "moo",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_a",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(Identifier {"int32"})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_b",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {ast.store(Identifier {"int32"})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_c",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {ast.store(Identifier {"int32"})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_d",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {ast.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {ast.store(Identifier {"int32"})},
									})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_e",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {ast.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {ast.store(Identifier {"int32"})},
									})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_f",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {ast.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {ast.store(Identifier {"int32"})},
									})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_g",
						   .type	  = ast.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {ast.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {ast.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {ast.store(Identifier {"int32"})},
									})},
								})},
							}),
					   }},
		.outputs	= {},
		.statements = {},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}

CERO_TEST(ParseVariableWithGenericType)
{
	auto source = make_test_source(R"_____(
bark()
{
	let List<int32> _a = ()
	let List<List<int32>> _b = ()
	let List<List<int32> > _c = ()
	let List<List<List<int32> > > _d = ()
	let List<List<List<int32> >> _e = ()
	let List<List<List<int32>> > _f = ()
	let List<List<List<int32>>> _g = ()
}
)_____");

	ExhaustiveReporter r;
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "bark",
		.parameters = {},
		.outputs	= {},
		.statements = {ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_a",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(Identifier {"int32"})},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_b",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {ast.store(Identifier {"int32"})},
								  })},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_c",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {ast.store(Identifier {"int32"})},
								  })},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_d",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {ast.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {ast.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_e",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {ast.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {ast.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_f",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {ast.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {ast.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_g",
						   .type		= ast.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {ast.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {ast.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {ast.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = ast.store(Call {
							   .arguments = {},
						   }),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}
