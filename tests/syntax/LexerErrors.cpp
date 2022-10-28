#include "util/ExhaustiveReporter.hpp"

#include <doctest/doctest.h>

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

TEST_CASE("NameTooLong")
{
	std::string_view repeated = "abcdefghijklmnopqrstuvwxyz";

	std::string source = "static bool ";
	source.reserve(16778000);
	for (size_t i = 0; i != 645278; ++i)
		source.append(repeated);

	ExhaustiveReporter r(std::move(source));
	CHECK(r.pop_report(Message::TokenTooLong, {1, 16777241}));
}

constexpr std::string_view REPEATED = "Water. Earth. Fire. Air. Long ago, the four nations lived together in harmony. Then, "
									  "everything changed when the Fire Nation attacked. Only the Avatar, master of all four "
									  "elements, could stop them, but when the world needed him most, he vanished. A hundred "
									  "years passed and my brother and I discovered the new Avatar, an airbender named Aang. "
									  "And although his airbending skills are great, he has a lot to learn before he's ready "
									  "to save anyone. But I believe Aang can save the world...";

TEST_CASE("StringTooLong")
{
	std::string source = "static String = \"";
	source.reserve(16778000);
	for (size_t i = 0; i != 34593; ++i)
		source.append(REPEATED);

	source.append("\"");

	ExhaustiveReporter r(std::move(source));
	CHECK(r.pop_report(Message::TokenTooLong, {1, 16777623}));
}

TEST_CASE("CommentTooLong")
{
	std::string source = "// ";
	source.reserve(16778000);
	for (size_t i = 0; i != 34593; ++i)
		source.append(REPEATED);

	ExhaustiveReporter r(std::move(source));
	CHECK(r.pop_report(Message::TokenTooLong, {1, 16777609}));
}
