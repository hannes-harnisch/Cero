#include "syntax/ParseExhaustive.hpp"
#include "util/Test.hpp"

TEST(AstSimpleMain)
{
	auto src	= make_test_source(R"_____(
main()
{}
)_____");
	auto result = parse_exhaustive(src);

	SyntaxTree expected;

	auto main = expected.add(Function("main", {}, {}, {}));
	expected.add_to_root(main);

	CHECK(result == expected);
}