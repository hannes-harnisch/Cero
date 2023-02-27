#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

using namespace cero::ast;

CERO_TEST(ParseGenericReturnType)
{
	auto source = make_test_source(R"_____(
a() -> List<List<int32> >
{
	return ();
}

b() -> List<List<int32>>
{
	return ();
}

c() -> List<List<List<int32> > >
{
	return ();
}

d() -> List<List<List<int32> >>
{
	return ();
}

e() -> List<List<List<int32>> >
{
	return ();
}

f() -> List<List<List<int32>>>
{
	return ();
}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "a",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {b.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {b.store(Identifier {"int32"})},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	b.add_to_root(b.store(Function {
		.name		= "b",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {b.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {b.store(Identifier {"int32"})},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	b.add_to_root(b.store(Function {
		.name		= "c",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {b.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {b.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {b.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	b.add_to_root(b.store(Function {
		.name		= "d",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {b.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {b.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {b.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	b.add_to_root(b.store(Function {
		.name		= "e",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {b.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {b.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {b.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	b.add_to_root(b.store(Function {
		.name		= "f",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "List",
				   .arguments = {b.store(GenericIdentifier {
					   .name	  = "List",
					   .arguments = {b.store(GenericIdentifier {
						   .name	  = "List",
						   .arguments = {b.store(Identifier {"int32"})},
					   })},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}

CERO_TEST(ParseLessAndRightShift)
{
	auto source = make_test_source(R"_____(
oof(int32 a, int32 b) -> bool
{
	return a < b >> (16 - 4);
}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "oof",
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
			   .type = b.store(Identifier {"bool"}),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(BinaryExpression {
				.op	   = BinaryOperator::Less,
				.left  = b.store(Identifier {"a"}),
				.right = b.store(BinaryExpression {
					.op	   = BinaryOperator::RightShift,
					.left  = b.store(Identifier {"b"}),
					.right = b.store(Call {
						.arguments = {b.store(BinaryExpression {
							.op	   = BinaryOperator::Subtract,
							.left  = b.store(NumericLiteral {
								 .kind = Literal::Decimal,
							 }),
							.right = b.store(NumericLiteral {
								.kind = Literal::Decimal,
							}),
						})},
					}),
				}),
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}

CERO_TEST(ParseAmbiguousGenericCallVsComparisonArguments)
{
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64
{
	return a(b<c, d>(e));
}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "ouch",
		.parameters = {Function::Parameter {
			.specifier = ParameterSpecifier::In,
			.name	   = "e",
			.type	   = b.store(Identifier {"float32"}),
		}},
		.outputs	= {FunctionOutput {
			   .type = b.store(Identifier {"float64"}),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.callee	   = b.store(Identifier {"a"}),
				.arguments = {b.store(Call {
					.callee	   = b.store(GenericIdentifier {
						   .name	  = "b",
						   .arguments = {b.store(Identifier {"c"}), b.store(Identifier {"d"})},
					   }),
					.arguments = {b.store(Identifier {"e"})},
				})},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}

CERO_TEST(ParseComparisonArgumentsVsGenericPattern)
{
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64
{
	return a(b < c, d > e);
}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "ouch",
		.parameters = {Function::Parameter {
			.specifier = ParameterSpecifier::In,
			.name	   = "e",
			.type	   = b.store(Identifier {"float32"}),
		}},
		.outputs	= {FunctionOutput {
			   .type = b.store(Identifier {"float64"}),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.callee	   = b.store(Identifier {"a"}),
				.arguments = {b.store(BinaryExpression {
								  .op	 = BinaryOperator::Less,
								  .left	 = b.store(Identifier {"b"}),
								  .right = b.store(Identifier {"c"}),
							  }),
							  b.store(BinaryExpression {
								  .op	 = BinaryOperator::Greater,
								  .left	 = b.store(Identifier {"d"}),
								  .right = b.store(Identifier {"e"}),
							  })},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}

CERO_TEST(ParseComparisonAndRightShiftAsGenericArgument)
{
	auto source = make_test_source(R"_____(
woof() -> A<(B > C)>
{
	return ();
}

meow() -> A<(D >> E)>
{
	return ();
}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "woof",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "A",
				   .arguments = {b.store(Call {
					   .arguments = {b.store(BinaryExpression {
						   .op	  = BinaryOperator::Greater,
						   .left  = b.store(Identifier {"B"}),
						   .right = b.store(Identifier {"C"}),
					   })},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	b.add_to_root(b.store(Function {
		.name		= "meow",
		.parameters = {},
		.outputs	= {FunctionOutput {
			   .type = b.store(GenericIdentifier {
				   .name	  = "A",
				   .arguments = {b.store(Call {
					   .arguments = {b.store(BinaryExpression {
						   .op	  = BinaryOperator::RightShift,
						   .left  = b.store(Identifier {"D"}),
						   .right = b.store(Identifier {"E"}),
					   })},
				   })},
			   }),
		   }},
		.statements = {b.store(Return {
			.expression = b.store(Call {
				.arguments = {},
			}),
		})},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
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
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "moo",
		.parameters = {Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_a",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(Identifier {"int32"})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_b",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {b.store(Identifier {"int32"})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_c",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {b.store(Identifier {"int32"})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_d",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {b.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {b.store(Identifier {"int32"})},
									})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_e",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {b.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {b.store(Identifier {"int32"})},
									})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_f",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {b.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {b.store(Identifier {"int32"})},
									})},
								})},
							}),
					   },
					   Function::Parameter {
						   .specifier = ParameterSpecifier::In,
						   .name	  = "_g",
						   .type	  = b.store(GenericIdentifier {
									.name	   = "List",
									.arguments = {b.store(GenericIdentifier {
										.name	   = "List",
										.arguments = {b.store(GenericIdentifier {
											.name	   = "List",
											.arguments = {b.store(Identifier {"int32"})},
									})},
								})},
							}),
					   }},
		.outputs	= {},
		.statements = {},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}

CERO_TEST(ParseVariableWithGenericType)
{
	auto source = make_test_source(R"_____(
bark()
{
	let List<int32> _a = ();
	let List<List<int32>> _b = ();
	let List<List<int32> > _c = ();
	let List<List<List<int32> > > _d = ();
	let List<List<List<int32> >> _e = ();
	let List<List<List<int32>> > _f = ();
	let List<List<List<int32>>> _g = ();
}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "bark",
		.parameters = {},
		.outputs	= {},
		.statements = {b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_a",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(Identifier {"int32"})},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_b",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {b.store(Identifier {"int32"})},
								  })},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_c",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {b.store(Identifier {"int32"})},
								  })},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_d",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {b.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {b.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_e",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {b.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {b.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_f",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {b.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {b.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Let,
						   .name		= "_g",
						   .type		= b.store(GenericIdentifier {
									  .name		 = "List",
									  .arguments = {b.store(GenericIdentifier {
										  .name		 = "List",
										  .arguments = {b.store(GenericIdentifier {
											  .name		 = "List",
											  .arguments = {b.store(Identifier {"int32"})},
									  })},
								  })},
							  }),
						   .initializer = b.store(Call {
							   .arguments = {},
						   }),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}
