#include "AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Parse.hpp>

namespace tests {

CERO_TEST(ParseAdditiveAndMultiplicativeOperators) {
	auto source = make_test_source(R"_____(
foo(int32 a, int32 b) -> int32 {
	let c = a + b;
	let d = a + b * c;
	let e = (d - a) / c;
	return e**2 * b;
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);
	CHECK(!ast.has_errors());

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "foo", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "a", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "b", [&] {
			c.name_expr("int32");
		});
		c.function_output("", [&] {
			c.name_expr("int32");
		});
		c.binding_statement(cero::BindingSpecifier::Let, "c", [&] {
			c.binary_expr(cero::BinaryOperator::Add, [&] {
				c.name_expr("a");
				c.name_expr("b");
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "d", [&] {
			c.binary_expr(cero::BinaryOperator::Add, [&] {
				c.name_expr("a");
				c.binary_expr(cero::BinaryOperator::Mul, [&] {
					c.name_expr("b");
					c.name_expr("c");
				});
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "e", [&] {
			c.binary_expr(cero::BinaryOperator::Div, [&] {
				c.group_expr([&] {
					c.binary_expr(cero::BinaryOperator::Sub, [&] {
						c.name_expr("d");
						c.name_expr("a");
					});
				});
				c.name_expr("c");
			});
		});
		c.return_expr([&] {
			c.binary_expr(cero::BinaryOperator::Mul, [&] {
				c.binary_expr(cero::BinaryOperator::Pow, [&] {
					c.name_expr("e");
					c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
				});
				c.name_expr("b");
			});
		});
	});

	c.compare();
}

CERO_TEST(ParseAdditiveAndComparisonOperators) {
	auto source = make_test_source(R"_____(
bar(int32 a, int32 b, int32 c) -> bool {
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
	auto ast = cero::parse(source, r);
	CHECK(!ast.has_errors());

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "bar", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "a", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "b", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "c", [&] {
			c.name_expr("int32");
		});
		c.function_output("", [&] {
			c.name_expr("bool");
		});
		c.binding_statement(cero::BindingSpecifier::Let, "u", [&] {
			c.binary_expr(cero::BinaryOperator::Eq, [&] {
				c.binary_expr(cero::BinaryOperator::Sub, [&] {
					c.name_expr("a");
					c.name_expr("b");
				});
				c.binary_expr(cero::BinaryOperator::Add, [&] {
					c.name_expr("b");
					c.name_expr("c");
				});
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "v", [&] {
			c.binary_expr(cero::BinaryOperator::NotEq, [&] {
				c.binary_expr(cero::BinaryOperator::Mul, [&] {
					c.name_expr("b");
					c.name_expr("a");
				});
				c.binary_expr(cero::BinaryOperator::Div, [&] {
					c.name_expr("c");
					c.name_expr("a");
				});
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "w", [&] {
			c.binary_expr(cero::BinaryOperator::Greater, [&] {
				c.binary_expr(cero::BinaryOperator::Add, [&] {
					c.name_expr("c");
					c.name_expr("b");
				});
				c.binary_expr(cero::BinaryOperator::Mul, [&] {
					c.name_expr("b");
					c.name_expr("a");
				});
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "x", [&] {
			c.binary_expr(cero::BinaryOperator::Less, [&] {
				c.binary_expr(cero::BinaryOperator::Div, [&] {
					c.name_expr("b");
					c.name_expr("a");
				});
				c.binary_expr(cero::BinaryOperator::Sub, [&] {
					c.name_expr("c");
					c.name_expr("b");
				});
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "y", [&] {
			c.binary_expr(cero::BinaryOperator::LessEq, [&] {
				c.binary_expr(cero::BinaryOperator::Mul, [&] {
					c.name_expr("a");
					c.name_expr("c");
				});
				c.binary_expr(cero::BinaryOperator::Sub, [&] {
					c.name_expr("b");
					c.name_expr("a");
				});
			});
		});
		c.binding_statement(cero::BindingSpecifier::Let, "z", [&] {
			c.binary_expr(cero::BinaryOperator::GreaterEq, [&] {
				c.binary_expr(cero::BinaryOperator::Add, [&] {
					c.name_expr("b");
					c.name_expr("c");
				});
				c.binary_expr(cero::BinaryOperator::Div, [&] {
					c.name_expr("a");
					c.name_expr("c");
				});
			});
		});
		c.return_expr([&] {
			c.binary_expr(cero::BinaryOperator::LogicOr, [&] {
				c.binary_expr(cero::BinaryOperator::LogicOr, [&] {
					c.binary_expr(cero::BinaryOperator::LogicOr, [&] {
						c.binary_expr(cero::BinaryOperator::LogicOr, [&] {
							c.binary_expr(cero::BinaryOperator::LogicOr, [&] {
								c.name_expr("u");
								c.name_expr("v");
							});
							c.name_expr("w");
						});
						c.name_expr("x");
					});
					c.name_expr("y");
				});
				c.name_expr("z");
			});
		});
	});

	c.compare();
}

CERO_TEST(ParseComparisonAndLogicalOperators) {
	auto source = make_test_source(R"_____(
baz(int32 a, int32 b, int32 c, int32 d) -> bool {
	return a + b == b + c && b + c != c + d && a < c && a > d;
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);
	CHECK(!ast.has_errors());

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "baz", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "a", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "b", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "c", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "d", [&] {
			c.name_expr("int32");
		});
		c.function_output("", [&] {
			c.name_expr("bool");
		});
		c.return_expr([&] {
			c.binary_expr(cero::BinaryOperator::LogicAnd, [&] {
				c.binary_expr(cero::BinaryOperator::LogicAnd, [&] {
					c.binary_expr(cero::BinaryOperator::LogicAnd, [&] {
						c.binary_expr(cero::BinaryOperator::Eq, [&] {
							c.binary_expr(cero::BinaryOperator::Add, [&] {
								c.name_expr("a");
								c.name_expr("b");
							});
							c.binary_expr(cero::BinaryOperator::Add, [&] {
								c.name_expr("b");
								c.name_expr("c");
							});
						});
						c.binary_expr(cero::BinaryOperator::NotEq, [&] {
							c.binary_expr(cero::BinaryOperator::Add, [&] {
								c.name_expr("b");
								c.name_expr("c");
							});
							c.binary_expr(cero::BinaryOperator::Add, [&] {
								c.name_expr("c");
								c.name_expr("d");
							});
						});
					});
					c.binary_expr(cero::BinaryOperator::Less, [&] {
						c.name_expr("a");
						c.name_expr("c");
					});
				});
				c.binary_expr(cero::BinaryOperator::Greater, [&] {
					c.name_expr("a");
					c.name_expr("d");
				});
			});
		});
	});

	c.compare();
}

} // namespace tests
