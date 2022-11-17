#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Token.hpp>

TEST(SourceTooLarge)
{
	ExhaustiveReporter r(std::string(16779000, ' '));
	CHECK(r.pop_report(Message::SourceInputTooLarge, {1, 1, test_name()}, Token::MAX_LENGTH));
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
}
)_____");
	CHECK(r.pop_report(Message::MissingClosingQuote, {4, 27, test_name()}));
	CHECK(r.pop_report(Message::MissingClosingQuote, {5, 16, test_name()}));
}

TEST(EscapedNonKeyword)
{
	ExhaustiveReporter r(R"_____(
\foo()
{}

\()
{}
)_____");
	CHECK(r.pop_report(Message::EscapedNonKeyword, {2, 2, test_name()}, "foo"));
	CHECK(r.pop_report(Message::EscapedNonKeyword, {5, 2, test_name()}, ""));
}

TEST(UnterminatedBlockComment)
{
	ExhaustiveReporter r(R"_____(
/* abc
)_____");
	CHECK(r.pop_report(Message::UnterminatedBlockComment, {2, 3, test_name()}));
}
