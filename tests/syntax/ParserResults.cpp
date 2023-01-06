#include "syntax/ParseExhaustive.hpp"
#include "util/Test.hpp"

TEST(AstForSimpleMain)
{
	auto src	= make_test_source(R"_____(
main()
{}
)_____");
	auto result = parse_exhaustive(src);

	cero::SyntaxTree expected;

	auto main = expected.add(cero::ast::Function("main", {}, {}, {}));
	expected.add_to_root(main);

	CHECK(result == expected);
}
