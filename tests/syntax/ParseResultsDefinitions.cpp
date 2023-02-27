#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

using namespace cero::ast;

CERO_TEST(ParseEmptyFunction)
{
	auto source = make_test_source(R"_____(
main()
{}
)_____");

	ExhaustiveReporter r;
	cero::AstBuilder   b;
	b.add_to_root(b.store(Function {
		.name		= "main",
		.parameters = {},
		.outputs	= {},
		.statements = {},
	}));

	auto result = cero::parse(source, r);
	CHECK(result == cero::SyntaxTree(b));
}
