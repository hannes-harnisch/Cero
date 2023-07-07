#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <syntax/LexicalToken.hpp>

CERO_TEST(SourceTooLarge) {
	ExhaustiveReporter r;
	r.expect(1, 1, cero::Message::SourceInputTooLarge, cero::LexicalToken::MaxLength);
	build_test_source(r, std::string(16779000, ' '));
}

CERO_TEST(IllegalChar) {
	ExhaustiveReporter r;
	r.expect(5, 1, cero::Message::UnexpectedCharacter, 0x7);
	build_test_source(r, R"_____(
main()
{}

() {}
)_____");
}

CERO_TEST(MissingClosingQuote) {
	ExhaustiveReporter r;
	r.expect(4, 28, cero::Message::MissingClosingQuote);
	r.expect(5, 17, cero::Message::MissingClosingQuote);
	r.expect(5, 5, cero::Message::ExpectSemicolon, "`let`");
	build_test_source(r, R"_____(
foo()
{
	let string = "Oh no...;
	let ch = 'x;
}
)_____");
}

CERO_TEST(UnterminatedBlockComment) {
	ExhaustiveReporter r;
	r.expect(2, 3, cero::Message::UnterminatedBlockComment);
	build_test_source(r, R"_____(
/* abc
)_____");
}
