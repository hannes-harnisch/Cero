#include "util/ExhaustiveReporter.hpp"

#include <doctest/doctest.h>

TEST_CASE("SourceTooLarge")
{
	ExhaustiveReporter r(std::string(16779000, ' '));
	CHECK(r.pop_report(Message::SourceInputTooLarge, {1, 1}));
}

TEST_CASE("IllegalChar")
{
	ExhaustiveReporter r(R"_____(
main() -> i32
{
	return 0 
}
)_____");
	CHECK(r.pop_report(Message::IllegalChar, {5, 2}, 7));
}

TEST_CASE("MissingClosingQuote")
{
	ExhaustiveReporter r(R"_____(
foo()
{
	let string = "Oh no...
	let ch = 'x
}
)_____");
	CHECK(r.pop_report(Message::MissingClosingQuote, {4, 24}));
	CHECK(r.pop_report(Message::MissingClosingQuote, {5, 13}));
}

TEST_CASE("EscapedNonKeyword")
{
	ExhaustiveReporter r(R"_____(
\foo()
{}
)_____");
	CHECK(r.pop_report(Message::EscapedNonKeyword, {2, 2}, "foo"));
}
