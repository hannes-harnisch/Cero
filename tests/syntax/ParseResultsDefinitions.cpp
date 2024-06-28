#include "AstCompare.hpp"
#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Parse.hpp>

namespace tests {

CERO_TEST(ParseEmptyFunction) {
	auto source = make_test_source(R"_____(
main() {
}
)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);
	CHECK(!ast.has_errors());

	AstCompare c(ast);
	c.root();
	c.function_definition(cero::AccessSpecifier::None, "main", [] {});

	c.compare();
}

} // namespace tests
