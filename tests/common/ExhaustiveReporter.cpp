#include "ExhaustiveReporter.hpp"

#include <doctest/doctest.h>
#include <util/Traits.hpp>

ExhaustiveReporter::ExhaustiveReporter(std::source_location location) :
	test_name(cero::normalize_function_name(location)) {
}

ExhaustiveReporter::~ExhaustiveReporter() {
	CHECK(expected_reports.empty());
}

void ExhaustiveReporter::write_report(cero::Message message,
									  cero::Severity,
									  cero::SourceLocation location,
									  std::format_args args) {
	auto format = cero::get_message_format(message);
	auto message_text = std::vformat(format, args);

	REQUIRE(!expected_reports.empty());

	auto& expected = expected_reports.front();
	Report received {message, location, message_text};
	const bool matches = expected == received;
	CHECK(matches);

	if (matches)
		expected_reports.pop();
}

void ExhaustiveReporter::on_expect(uint32_t line, uint32_t column, cero::Message message, std::format_args args) {
	auto format = cero::get_message_format(message);
	auto message_text = std::vformat(format, args);

	cero::SourceLocation location {test_name, line, column};
	expected_reports.emplace(message, location, std::move(message_text));
}
