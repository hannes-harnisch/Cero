#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/LexicalToken.hpp>

TEST(SourceTooLarge)
{
	ExhaustiveReporter r(std::string(16779000, ' '));
	CHECK(r.pop_report(Message::SourceInputTooLarge, {1, 1, test_name()}, LexicalToken::MAX_LENGTH));
}

TEST(IllegalChar)
{
	ExhaustiveReporter r(R"_____(
main()
{}

() {}
)_____");
	CHECK(r.pop_report(Message::UnexpectedCharacter, {5, 1, test_name()}, 7));
}

TEST(MissingClosingQuote)
{
	ExhaustiveReporter r(R"_____(
foo()
{
	let string = "Oh no...
	let ch = 'x

	a + b * c >> d
}
)_____");
	CHECK(r.pop_report(Message::MissingClosingQuote, {4, 27, test_name()}));
	CHECK(r.pop_report(Message::MissingClosingQuote, {5, 16, test_name()}));
}

TEST(UnterminatedBlockComment)
{
	ExhaustiveReporter r(R"_____(
/* abc
)_____");
	CHECK(r.pop_report(Message::UnterminatedBlockComment, {2, 3, test_name()}));
}
