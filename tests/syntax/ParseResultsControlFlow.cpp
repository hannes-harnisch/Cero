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
	cero::SyntaxTree   ast;
	ast.add_to_root(ast.store(Function {
		.name		= "fibonacci",
		.parameters = {Function::Parameter {
			.specifier = ParameterSpecifier::Var,
			.name	   = "n",
			.type	   = ast.store(Identifier {"uint32"}),
		}},
		.outputs	= {FunctionOutput {
			   .type = ast.store(Identifier {"uint32"}),
		   }},
		.statements = {ast.store(Binding {
						   .specifier	= Binding::Specifier::Var,
						   .name		= "result",
						   .type		= ast.store(Identifier {"uint32"}),
						   .initializer = ast.store(NumericLiteral {
							   .kind = Literal::Decimal,
						   }),
					   }),
					   ast.store(Binding {
						   .specifier	= Binding::Specifier::Var,
						   .name		= "next",
						   .type		= ast.store(Identifier {"uint32"}),
						   .initializer = ast.store(NumericLiteral {
							   .kind = Literal::Decimal,
						   }),
					   }),
					   ast.store(WhileLoop {
						   .condition = ast.store(BinaryExpression {
							   .op	  = BinaryOperator::NotEqual,
							   .left  = ast.store(UnaryExpression {
									.op		 = UnaryOperator::PostDecrement,
									.operand = ast.store(Identifier {"n"}),
								}),
							   .right = ast.store(NumericLiteral {
								   .kind = Literal::Decimal,
							   }),
						   }),
						   .statement = ast.store(Block {
							   .statements = {ast.store(Binding {
												  .specifier   = Binding::Specifier::Let,
												  .name		   = "temp",
												  .initializer = ast.store(Identifier {"next"}),
											  }),
											  ast.store(BinaryExpression {
												  .op	 = BinaryOperator::Assign,
												  .left	 = ast.store(Identifier {"next"}),
												  .right = ast.store(Identifier {"result"}),
											  }),
											  ast.store(BinaryExpression {
												  .op	 = BinaryOperator::AddAssign,
												  .left	 = ast.store(Identifier {"result"}),
												  .right = ast.store(Identifier {"temp"}),
											  })},
						   }),
					   }),
					   ast.store(Return {
						   .expression = ast.store(Identifier {"result"}),
					   })},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == ast);
}
