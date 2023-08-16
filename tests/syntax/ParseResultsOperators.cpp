#include "common/AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <syntax/Parse.hpp>

CERO_TEST(ParseAdditiveAndMultiplicativeOperators) {
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

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "foo");
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "a");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "b");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	{
		auto _1 = c.mark_children();

		c.add_binding_statement(cero::BindingSpecifier::Let, "c");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Add);
			{
				auto _3 = c.mark_children();
				c.add_name_expr("a");
				c.add_name_expr("b");
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "d");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Add);
			{
				auto _3 = c.mark_children();
				c.add_name_expr("a");
				c.add_binary_expr(cero::BinaryOperator::Multiply);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("c");
				}
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "e");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Divide);
			{
				auto _3 = c.mark_children();
				c.add_group_expr();
				{
					auto _4 = c.mark_children();
					c.add_binary_expr(cero::BinaryOperator::Subtract);
					{
						auto _5 = c.mark_children();
						c.add_name_expr("d");
						c.add_name_expr("a");
					}
				}
				c.add_name_expr("c");
			}
		}

		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Multiply);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Power);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("e");
					c.add_numeric_literal_expr(cero::NumericLiteralKind::Decimal);
				}
				c.add_name_expr("b");
			}
		}
	}

	c.compare();
}

CERO_TEST(ParseAdditiveAndComparisonOperators) {
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

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "bar");
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "a");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "b");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "c");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("bool");
	}
	{
		auto _1 = c.mark_children();

		c.add_binding_statement(cero::BindingSpecifier::Let, "u");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Equal);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Subtract);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("a");
					c.add_name_expr("b");
				}
				c.add_binary_expr(cero::BinaryOperator::Add);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("c");
				}
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "v");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::NotEqual);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Multiply);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("a");
				}
				c.add_binary_expr(cero::BinaryOperator::Divide);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("c");
					c.add_name_expr("a");
				}
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "w");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Greater);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Add);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("c");
					c.add_name_expr("b");
				}
				c.add_binary_expr(cero::BinaryOperator::Multiply);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("a");
				}
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "x");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Less);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Divide);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("a");
				}
				c.add_binary_expr(cero::BinaryOperator::Subtract);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("c");
					c.add_name_expr("b");
				}
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "y");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::LessEqual);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Multiply);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("a");
					c.add_name_expr("c");
				}
				c.add_binary_expr(cero::BinaryOperator::Subtract);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("a");
				}
			}
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "z");
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::GreaterEqual);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Add);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("c");
				}
				c.add_binary_expr(cero::BinaryOperator::Divide);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("a");
					c.add_name_expr("c");
				}
			}
		}

		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::LogicalOr);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::LogicalOr);
				{
					auto _4 = c.mark_children();
					c.add_binary_expr(cero::BinaryOperator::LogicalOr);
					{
						auto _5 = c.mark_children();
						c.add_binary_expr(cero::BinaryOperator::LogicalOr);
						{
							auto _6 = c.mark_children();
							c.add_binary_expr(cero::BinaryOperator::LogicalOr);
							{
								auto _7 = c.mark_children();
								c.add_name_expr("u");
								c.add_name_expr("v");
							}
							c.add_name_expr("w");
						}
						c.add_name_expr("x");
					}
					c.add_name_expr("y");
				}
				c.add_name_expr("z");
			}
		}
	}

	c.compare();
}

CERO_TEST(ParseComparisonAndLogicalOperators) {
	auto source = make_test_source(R"_____(
baz(int32 a, int32 b, int32 c, int32 d) -> bool
{
    return a + b == b + c && b + c != c + d && a < c && a > d;
}
)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "baz");
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "a");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "b");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "c");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "d");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("int32");
	}
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("bool");
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::LogicalAnd);
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::LogicalAnd);
				{
					auto _4 = c.mark_children();
					c.add_binary_expr(cero::BinaryOperator::LogicalAnd);
					{
						auto _5 = c.mark_children();
						c.add_binary_expr(cero::BinaryOperator::Equal);
						{
							auto _6 = c.mark_children();
							c.add_binary_expr(cero::BinaryOperator::Add);
							{
								auto _7 = c.mark_children();
								c.add_name_expr("a");
								c.add_name_expr("b");
							}
							c.add_binary_expr(cero::BinaryOperator::Add);
							{
								auto _7 = c.mark_children();
								c.add_name_expr("b");
								c.add_name_expr("c");
							}
						}
						c.add_binary_expr(cero::BinaryOperator::NotEqual);
						{
							auto _6 = c.mark_children();
							c.add_binary_expr(cero::BinaryOperator::Add);
							{
								auto _7 = c.mark_children();
								c.add_name_expr("b");
								c.add_name_expr("c");
							}
							c.add_binary_expr(cero::BinaryOperator::Add);
							{
								auto _7 = c.mark_children();
								c.add_name_expr("c");
								c.add_name_expr("d");
							}
						}
					}
					c.add_binary_expr(cero::BinaryOperator::Less);
					{
						auto _5 = c.mark_children();
						c.add_name_expr("a");
						c.add_name_expr("c");
					}
				}
				c.add_binary_expr(cero::BinaryOperator::Greater);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("a");
					c.add_name_expr("d");
				}
			}
		}
	}

	c.compare();
}
