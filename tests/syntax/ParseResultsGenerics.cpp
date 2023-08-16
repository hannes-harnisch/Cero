#include "common/AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <syntax/Parse.hpp>

CERO_TEST(ParseGenericReturnType) {
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

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "a");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_name_expr("int32");
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

	c.add_function_definition(cero::AccessSpecifier::None, "b");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_name_expr("int32");
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

	c.add_function_definition(cero::AccessSpecifier::None, "c");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

	c.add_function_definition(cero::AccessSpecifier::None, "d");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

	c.add_function_definition(cero::AccessSpecifier::None, "e");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

	c.add_function_definition(cero::AccessSpecifier::None, "f");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

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
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "oof");
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
		c.add_name_expr("bool");
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_binary_expr(cero::BinaryOperator::Less);
			{
				auto _3 = c.mark_children();
				c.add_name_expr("a");
				c.add_binary_expr(cero::BinaryOperator::RightShift);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_group_expr();
					{
						auto _5 = c.mark_children();
						c.add_binary_expr(cero::BinaryOperator::Subtract);
						{
							auto _6 = c.mark_children();
							c.add_numeric_literal_expr(cero::NumericLiteralKind::Decimal);
							c.add_numeric_literal_expr(cero::NumericLiteralKind::Decimal);
						}
					}
				}
			}
		}
	}

	c.compare();
}

CERO_TEST(ParseAmbiguousGenericCallVsComparisonArguments) {
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64
{
	return a(b<c, d>(e));
}
)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "ouch");
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "e");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("float32");
	}
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("float64");
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_call_expr();
			{
				auto _3 = c.mark_children();
				c.add_name_expr("a");
				c.add_call_expr();
				{
					auto _4 = c.mark_children();
					c.add_generic_name_expr("b");
					{
						auto _5 = c.mark_children();
						c.add_name_expr("c");
						c.add_name_expr("d");
					}
					c.add_name_expr("e");
				}
			}
		}
	}

	c.compare();
}

CERO_TEST(ParseComparisonArgumentsVsGenericPattern) {
	auto source = make_test_source(R"_____(
ouch(float32 e) -> float64
{
	return a(b < c, d > e);
}
)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "ouch");
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "e");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("float32");
	}
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("float64");
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_call_expr();
			{
				auto _3 = c.mark_children();
				c.add_name_expr("a");
				c.add_binary_expr(cero::BinaryOperator::Less);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("b");
					c.add_name_expr("c");
				}
				c.add_binary_expr(cero::BinaryOperator::Greater);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("d");
					c.add_name_expr("e");
				}
			}
		}
	}

	c.compare();
}

CERO_TEST(ParseComparisonAndRightShiftAsGenericArgument) {
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

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "woof");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("A");
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::Greater);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("B");
					c.add_name_expr("C");
				}
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

	c.add_function_definition(cero::AccessSpecifier::None, "meow");
	c.add_function_definition_output("");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("A");
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
			{
				auto _3 = c.mark_children();
				c.add_binary_expr(cero::BinaryOperator::RightShift);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("D");
					c.add_name_expr("E");
				}
			}
		}
	}
	{
		auto _1 = c.mark_children();
		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_group_expr();
		}
	}

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
	List<List<List<int32>>> _g)
{}
)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "moo");
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_a");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_name_expr("int32");
		}
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_b");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_name_expr("int32");
			}
		}
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_c");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_name_expr("int32");
			}
		}
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_d");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_e");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_f");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}
	c.add_function_definition_parameter(cero::ParameterSpecifier::None, "_g");
	{
		auto _1 = c.mark_children();
		c.add_generic_name_expr("List");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
		}
	}

	c.compare();
}

CERO_TEST(ParseVariableWithGenericType) {
	auto source = make_test_source(R"_____(
bark()
{
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
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "bark");
	{
		auto _1 = c.mark_children();

		c.add_binding_statement(cero::BindingSpecifier::Let, "_a");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_name_expr("int32");
			}
			c.add_group_expr();
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "_b");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
			c.add_group_expr();
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "_c");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_name_expr("int32");
				}
			}
			c.add_group_expr();
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "_d");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_generic_name_expr("List");
					{
						auto _5 = c.mark_children();
						c.add_name_expr("int32");
					}
				}
			}
			c.add_group_expr();
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "_e");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_generic_name_expr("List");
					{
						auto _5 = c.mark_children();
						c.add_name_expr("int32");
					}
				}
			}
			c.add_group_expr();
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "_f");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_generic_name_expr("List");
					{
						auto _5 = c.mark_children();
						c.add_name_expr("int32");
					}
				}
			}
			c.add_group_expr();
		}

		c.add_binding_statement(cero::BindingSpecifier::Let, "_g");
		{
			auto _2 = c.mark_children();
			c.add_generic_name_expr("List");
			{
				auto _3 = c.mark_children();
				c.add_generic_name_expr("List");
				{
					auto _4 = c.mark_children();
					c.add_generic_name_expr("List");
					{
						auto _5 = c.mark_children();
						c.add_name_expr("int32");
					}
				}
			}
			c.add_group_expr();
		}
	}

	c.compare();
}
