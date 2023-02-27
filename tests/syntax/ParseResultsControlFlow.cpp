#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

using namespace cero::ast;

CERO_TEST(ParseSimpleFunction)
{
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
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "fibonacci",
		.parameters = {Function::Parameter {
			.specifier = ParameterSpecifier::Var,
			.name	   = "n",
			.type	   = b.store(Identifier {"uint32"}),
		}},
		.outputs	= {FunctionOutput {
			   .type = b.store(Identifier {"uint32"}),
		   }},
		.statements = {b.store(Binding {
						   .specifier	= Binding::Specifier::Var,
						   .name		= "result",
						   .type		= b.store(Identifier {"uint32"}),
						   .initializer = b.store(NumericLiteral {
							   .kind = Literal::Decimal,
						   }),
					   }),
					   b.store(Binding {
						   .specifier	= Binding::Specifier::Var,
						   .name		= "next",
						   .type		= b.store(Identifier {"uint32"}),
						   .initializer = b.store(NumericLiteral {
							   .kind = Literal::Decimal,
						   }),
					   }),
					   b.store(WhileLoop {
						   .condition = b.store(BinaryExpression {
							   .op	  = BinaryOperator::NotEqual,
							   .left  = b.store(UnaryExpression {
									.op		 = UnaryOperator::PostDecrement,
									.operand = b.store(Identifier {"n"}),
								}),
							   .right = b.store(NumericLiteral {
								   .kind = Literal::Decimal,
							   }),
						   }),
						   .statement = b.store(Block {
							   .statements = {b.store(Binding {
												  .specifier   = Binding::Specifier::Let,
												  .name		   = "temp",
												  .initializer = b.store(Identifier {"next"}),
											  }),
											  b.store(BinaryExpression {
												  .op	 = BinaryOperator::Assign,
												  .left	 = b.store(Identifier {"next"}),
												  .right = b.store(Identifier {"result"}),
											  }),
											  b.store(BinaryExpression {
												  .op	 = BinaryOperator::AddAssign,
												  .left	 = b.store(Identifier {"result"}),
												  .right = b.store(Identifier {"temp"}),
											  })},
						   }),
					   }),
					   b.store(Return {
						   .expression = b.store(Identifier {"result"}),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}
