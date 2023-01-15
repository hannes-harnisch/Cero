#include "syntax/ParseExhaustive.hpp"
#include "util/Test.hpp"

TEST(AstForSimpleMain)
{
	auto src	= make_test_source(R"_____(
main()
{}
)_____");
	auto result = parse_exhaustive(src);

	cero::AstState ast;
	ast.add_to_root(ast.add(cero::ast::Function {
		.name		= "main",
		.parameters = {},
		.returns	= {},
		.statements = {},
	}));
	CHECK(result == cero::SyntaxTree(ast));
}

TEST(AstForFibonacci)
{
	auto src	= make_test_source(R"_____(
fibonacci(var usize n) -> usize
{
    var usize result = 0
    var usize next   = 1

    /*while n-- != 0
    {
        let temp = next
        next = result
        result += temp
    }*/

    return result
}
)_____");
	auto result = parse_exhaustive(src);

	cero::AstState ast;
	ast.add_to_root(ast.add(cero::ast::Function {
		.name		= "fibonacci",
		.parameters = {cero::ast::Function::Parameter {
			.specifier = cero::ast::ParameterSpecifier::Var,
			.name	   = "n",
			.type	   = ast.add(cero::ast::Identifier {
					 .name = "usize",
			 }),
		}},
		.returns	= {cero::ast::ReturnValue {
			   .type = ast.add(cero::ast::Identifier {
				   .name = "usize",
			   }),
		   }},
		.statements = {ast.add(cero::ast::Binding {
						   .specifier	= cero::ast::Binding::Specifier::Var,
						   .name		= "result",
						   .type		= ast.add(cero::ast::Identifier {
									  .name = "usize",
							  }),
						   .initializer = ast.add(cero::ast::NumericLiteral {
							   .kind = cero::ast::Literal::Decimal,
						   }),
					   }),
					   ast.add(cero::ast::Binding {
						   .specifier	= cero::ast::Binding::Specifier::Var,
						   .name		= "next",
						   .type		= ast.add(cero::ast::Identifier {
									  .name = "usize",
							  }),
						   .initializer = ast.add(cero::ast::NumericLiteral {
							   .kind = cero::ast::Literal::Decimal,
						   }),
					   }),
					   ast.add(cero::ast::Return {
						   .expression = ast.add(cero::ast::Identifier {
							   .name = "result",
						   }),
					   })},
	}));
	CHECK(result == cero::SyntaxTree(ast));
}
