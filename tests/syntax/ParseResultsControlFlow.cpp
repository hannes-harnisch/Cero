#include "AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Parse.hpp>

namespace tests {

CERO_TEST(ParseWhileLoopFibonacci) {
	auto source = make_test_source(R"_____(
fibonacci(var uint32 n) -> uint32 {
    var uint32 result = 0;
    var uint32 next   = 1;

    while n-- != 0 {
        let temp = next;
        next = result;
        result += temp;
    }

    return result;
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);
	CHECK(!ast.has_errors());

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "fibonacci", [&] {
		c.function_parameter(cero::ParameterSpecifier::Var, "n", [&] {
			c.name_expr("uint32");
		});
		c.function_output("", [&] {
			c.name_expr("uint32");
		});
		c.binding_statement(cero::BindingSpecifier::Var, "result", [&] {
			c.name_expr("uint32");
			c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
		});
		c.binding_statement(cero::BindingSpecifier::Var, "next", [&] {
			c.name_expr("uint32");
			c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
		});
		c.while_loop([&] {
			c.binary_expr(cero::BinaryOperator::NotEq, [&] {
				c.unary_expr(cero::UnaryOperator::PostDec, [&] {
					c.name_expr("n");
				});
				c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
			});
			c.binding_statement(cero::BindingSpecifier::Let, "temp", [&] {
				c.name_expr("next");
			});
			c.binary_expr(cero::BinaryOperator::Assign, [&] {
				c.name_expr("next");
				c.name_expr("result");
			});
			c.binary_expr(cero::BinaryOperator::AddAssign, [&] {
				c.name_expr("result");
				c.name_expr("temp");
			});
		});
		c.return_expr([&] {
			c.name_expr("result");
		});
	});

	c.compare();
}

CERO_TEST(ParseIfStatement) {
	auto source = make_test_source(R"_____(
public divChecked(int32 a, int32 b) -> Opt<int32> {
	if b == 0 {
		return null;
	} else {
		return a / b;
	}
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);
	CHECK(!ast.has_errors());

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::Public, "divChecked", [&] {
		c.function_parameter(cero::ParameterSpecifier::None, "a", [&] {
			c.name_expr("int32");
		});
		c.function_parameter(cero::ParameterSpecifier::None, "b", [&] {
			c.name_expr("int32");
		});
		c.function_output("", [&] {
			c.generic_name_expr("Opt", [&] {
				c.name_expr("int32");
			});
		});
		c.if_expr([&] {
			c.binary_expr(cero::BinaryOperator::Eq, [&] {
				c.name_expr("b");
				c.numeric_literal_expr(cero::NumericLiteralKind::Decimal);
			});
			c.return_expr([&] {
				c.name_expr("null");
			});
			c.return_expr([&] {
				c.binary_expr(cero::BinaryOperator::Div, [&] {
					c.name_expr("a");
					c.name_expr("b");
				});
			});
		});
	});

	c.compare();
}

} // namespace tests
