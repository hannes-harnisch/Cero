#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

namespace tests {

CERO_TEST(SourceTooLarge) {
	ExhaustiveReporter r;
	r.expect(0, 0, cero::Message::SourceInputTooLarge, cero::ReportArgs(cero::MaxSourceLength));
	build_test_source(r, std::string(cero::MaxSourceLength + 1, ' '));
}

CERO_TEST(IllegalChar) {
	ExhaustiveReporter r;
	r.expect(5, 1, cero::Message::UnexpectedCharacter, cero::ReportArgs(0x7));
	build_test_source(r, R"_____(
main() {
}

() {
}
)_____");
}

CERO_TEST(MissingClosingQuote) {
	ExhaustiveReporter r;
	r.expect(3, 28, cero::Message::MissingClosingQuote, {});
	r.expect(5, 17, cero::Message::MissingClosingQuote, {});
	build_test_source(r, R"_____(
foo() {
	let string = "Oh no...;
	;
	let ch = 'x;
	;
}
)_____");
}

CERO_TEST(UnterminatedBlockComment) {
	ExhaustiveReporter r;
	r.expect(2, 1, cero::Message::UnterminatedBlockComment, {});
	build_test_source(r, R"_____(
/* abc
)_____");
}

} // namespace tests
