#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/LexicalToken.hpp>

CERO_TEST(SourceTooLarge)
{
	ExhaustiveReporter r;
	r.expect(1, 1, cero::Message::SourceInputTooLarge, cero::LexicalToken::MAX_LENGTH);
	build_test_source(r, std::string(16779000, ' '));
}

CERO_TEST(IllegalChar)
{
	ExhaustiveReporter r;
	r.expect(5, 1, cero::Message::UnexpectedCharacter, 0x7);
	build_test_source(r, R"_____(
main()
{}

() {}
)_____");
}

CERO_TEST(MissingClosingQuote)
{
	ExhaustiveReporter r;
	r.expect(4, 27, cero::Message::MissingClosingQuote);
	r.expect(5, 16, cero::Message::MissingClosingQuote);
	build_test_source(r, R"_____(
foo()
{
	let string = "Oh no...
	let ch = 'x
}
)_____");
}

CERO_TEST(UnterminatedBlockComment)
{
	ExhaustiveReporter r;
	r.expect(2, 3, cero::Message::UnterminatedBlockComment);
	build_test_source(r, R"_____(
/* abc
)_____");
}
