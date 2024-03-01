#include "AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Parse.hpp>

namespace tests {

CERO_TEST(ParseGenericReturnType) {
	auto source = make_test_source(R"_____(
a() -> List<List<int32> > {
	return ();
}

b() -> List<List<int32>> {
	return ();
}

c() -> List<List<List<int32> > > {
	return ();
}

d() -> List<List<List<int32> >> {
	return ();
}

e() -> List<List<List<int32>> > {
	return ();
}

f() -> List<List<List<int32>>> {
	return ();
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "a", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.name_expr("int32");
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});
	c.function_definition(cero::AccessSpecifier::None, "b", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.name_expr("int32");
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});
	c.function_definition(cero::AccessSpecifier::None, "c", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});
	c.function_definition(cero::AccessSpecifier::None, "d", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});
	c.function_definition(cero::AccessSpecifier::None, "e", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});
	c.function_definition(cero::AccessSpecifier::None, "f", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});

	c.compare();
}

CERO_TEST(ParseLessAndRightShift) {
	auto source = make_test_source(R"_____(
oof(int32 a, int32 b) -> bool
{
	return a < b >> (16 - 4);
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "oof", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "a", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "b", [&] {
			c.name_expr("int32");
		});
		c.function_output("", [&] {
			c.name_expr("bool");
		});
		c.return_expr([&] {
			c.binary_expr(cero::BinaryOperator::Less, [&] {
				c.name_expr("a");
				c.binary_expr(cero::BinaryOperator::RightShift, [&] {
					c.name_expr("b");
					c.group_expr([&] {
						c.binary_expr(cero::BinaryOperator::Subtract, [&] {
							c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
							c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
						});
					});
				});
			});
		});
	});

	c.compare();
}

CERO_TEST(ParseAmbiguousGenericCallVsComparisonArguments) {
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64 {
	return a(b<c, d>(e));
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "ouch", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "e", [&] {
			c.name_expr("float32");
		});
		c.function_output("", [&] {
			c.name_expr("float64");
		});
		c.return_expr([&] {
			c.call_expr([&] {
				c.name_expr("a");
				c.call_expr([&] {
					c.generic_name_expr("b", [&] {
						c.name_expr("c");
						c.name_expr("d");
					});
					c.name_expr("e");
				});
			});
		});
	});

	c.compare();
}

CERO_TEST(ParseComparisonArgumentsVsGenericPattern) {
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64 {
	return a(b < c, d > e);
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "ouch", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "e", [&] {
			c.name_expr("float32");
		});
		c.function_output("", [&] {
			c.name_expr("float64");
		});
		c.return_expr([&] {
			c.call_expr([&] {
				c.name_expr("a");
				c.binary_expr(cero::BinaryOperator::Less, [&] {
					c.name_expr("b");
					c.name_expr("c");
				});
				c.binary_expr(cero::BinaryOperator::Greater, [&] {
					c.name_expr("d");
					c.name_expr("e");
				});
			});
		});
	});

	c.compare();
}

CERO_TEST(ParseComparisonAndRightShiftAsGenericArgument) {
	auto source = make_test_source(R"_____(
woof() -> A<(B > C)> {
	return ();
}

meow() -> A<(D >> E)> {
	return ();
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "woof", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("A", [&] {
				c.group_expr([&] {
					c.binary_expr(cero::BinaryOperator::Greater, [&] {
						c.name_expr("B");
						c.name_expr("C");
					});
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});
	c.function_definition(cero::AccessSpecifier::None, "meow", [&] {
		c.function_output("", [&] {
			c.generic_name_expr("A", [&] {
				c.group_expr([&] {
					c.binary_expr(cero::BinaryOperator::RightShift, [&] {
						c.name_expr("D");
						c.name_expr("E");
					});
				});
			});
		});
		c.return_expr([&] {
			c.group_expr([] {});
		});
	});

	c.compare();
}

CERO_TEST(ParseGenericParameters) {
	auto source = make_test_source(R"_____(
moo(List<int32> _a,
	List<List<int32>> _b,
	List<List<int32> > _c,
	List<List<List<int32> > > _d,
	List<List<List<int32> >> _e,
	List<List<List<int32>> > _f,
	List<List<List<int32>>> _g) {
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "moo", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "_a", [&] {
			c.generic_name_expr("List", [&] {
				c.name_expr("int32");
			});
		});
		c.function_parameter(cero::ParameterSpecifier::None, "_b", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.name_expr("int32");
				});
			});
		});
		c.function_parameter(cero::ParameterSpecifier::None, "_c", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.name_expr("int32");
				});
			});
		});
		c.function_parameter(cero::ParameterSpecifier::None, "_d", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.function_parameter(cero::ParameterSpecifier::None, "_e", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.function_parameter(cero::ParameterSpecifier::None, "_f", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
		c.function_parameter(cero::ParameterSpecifier::None, "_g", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
		});
	});

	c.compare();
}

CERO_TEST(ParseVariableWithGenericType) {
	auto source = make_test_source(R"_____(
bark() {
	List<int32> _a = ();
	List<List<int32>> _b = ();
	List<List<int32> > _c = ();
	List<List<List<int32> > > _d = ();
	List<List<List<int32> >> _e = ();
	List<List<List<int32>> > _f = ();
	List<List<List<int32>>> _g = ();
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "bark", [&] {
		c.binding_statement(cero::BindingSpecifier::Let, "_a", [&] {
			c.generic_name_expr("List", [&] {
				c.name_expr("int32");
			});
			c.group_expr([] {});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "_b", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.name_expr("int32");
				});
			});
			c.group_expr([] {});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "_c", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.name_expr("int32");
				});
			});
			c.group_expr([] {});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "_d", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
			c.group_expr([] {});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "_e", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
			c.group_expr([] {});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "_f", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
			c.group_expr([] {});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "_g", [&] {
			c.generic_name_expr("List", [&] {
				c.generic_name_expr("List", [&] {
					c.generic_name_expr("List", [&] {
						c.name_expr("int32");
					});
				});
			});
			c.group_expr([] {});
		});
	});

	c.compare();
}

} // namespace tests
