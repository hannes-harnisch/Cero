#include "util/ExhaustiveReporter.hpp"

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

TEST_CASE("NameTooLong")
{
	std::string_view path = "syntax/NameTooLong.ax";

	auto r = ExhaustiveReporter::from(path);
	CHECK(r.pop_report(Message::TokenTooLong, {1, 12, path}));
}

TEST_CASE("StringTooLong")
{
	std::string_view path = "syntax/StringTooLong.ax";

	auto r = ExhaustiveReporter::from(path);
	CHECK(r.pop_report(Message::TokenTooLong, {3, 9, path}));
}

TEST_CASE("CommentTooLong")
{
	std::string_view path = "syntax/CommentTooLong.ax";

	auto r = ExhaustiveReporter::from(path);
	CHECK(r.pop_report(Message::TokenTooLong, {1, 2, path}));
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
