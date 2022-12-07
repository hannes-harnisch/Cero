#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/LexicalToken.hpp>

TEST(SourceTooLarge)
{
	auto r = build_test_source(std::string(16779000, ' '));
	CHECK(r.pop_report(1, 1, Message::SourceInputTooLarge, LexicalToken::MAX_LENGTH));
}

TEST(IllegalChar)
{
	auto r = build_test_source(R"_____(
main()
{}

() {}
)_____");
	CHECK(r.pop_report(5, 1, Message::UnexpectedCharacter, 0x7));
}

TEST(MissingClosingQuote)
{
	auto r = build_test_source(R"_____(
foo()
{
	let string = "Oh no...
	let ch = 'x
}
)_____");
	CHECK(r.pop_report(4, 27, Message::MissingClosingQuote));
	CHECK(r.pop_report(5, 16, Message::MissingClosingQuote));
}

TEST(UnterminatedBlockComment)
{
	auto r = build_test_source(R"_____(
/* abc
)_____");
	CHECK(r.pop_report(2, 3, Message::UnterminatedBlockComment));
}
