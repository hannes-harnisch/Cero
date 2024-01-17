#include "AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Parse.hpp>

CERO_TEST(ParseSimpleFunction) {
	auto source = make_test_source(R"_____(
fibonacci(var uint32 n) -> uint32
{
    var uint32 result = 0;
    var uint32 next   = 1;

    while n-- != 0
    {
        let temp = next;
        next = result;
        result += temp;
    }

    return result;
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	AstCompare c(ast);
	c.add_root();

	c.add_function_definition(cero::AccessSpecifier::None, "fibonacci");
	c.add_function_parameter(cero::ParameterSpecifier::Var, "n");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("uint32");
	}
	c.add_function_output("");
	{
		auto _1 = c.mark_children();
		c.add_name_expr("uint32");
	}
	{
		auto _1 = c.mark_children();

		c.add_binding_statement(cero::BindingSpecifier::Var, "result");
		{
			auto _2 = c.mark_children();

			c.add_name_expr("uint32");
			c.add_numeric_literal_expr(cero::NumericLiteralKind::Decimal);
		}

		c.add_binding_statement(cero::BindingSpecifier::Var, "next");
		{
			auto _2 = c.mark_children();

			c.add_name_expr("uint32");
			c.add_numeric_literal_expr(cero::NumericLiteralKind::Decimal);
		}

		c.add_while_loop();
		{
			auto _2 = c.mark_children();

			c.add_binary_expr(cero::BinaryOperator::NotEqual);
			{
				auto _3 = c.mark_children();

				c.add_unary_expr(cero::UnaryOperator::PostDecrement);
				{
					auto _4 = c.mark_children();
					c.add_name_expr("n");
				}

				c.add_numeric_literal_expr(cero::NumericLiteralKind::Decimal);
			}

			c.add_binding_statement(cero::BindingSpecifier::Let, "temp");
			{
				auto _3 = c.mark_children();
				c.add_name_expr("next");
			}

			c.add_binary_expr(cero::BinaryOperator::Assign);
			{
				auto _3 = c.mark_children();
				c.add_name_expr("next");
				c.add_name_expr("result");
			}

			c.add_binary_expr(cero::BinaryOperator::AddAssign);
			{
				auto _3 = c.mark_children();
				c.add_name_expr("result");
				c.add_name_expr("temp");
			}
		}

		c.add_return_expr();
		{
			auto _2 = c.mark_children();
			c.add_name_expr("result");
		}
	}

	c.compare();
}
