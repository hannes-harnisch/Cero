#include "ExhaustiveReporter.hpp"

#include <cero/util/Traits.hpp>
#include <doctest/doctest.h>

ExhaustiveReporter::ExhaustiveReporter(std::source_location location) :
	test_name(cero::normalize_function_name(location))
{}

ExhaustiveReporter::~ExhaustiveReporter()
{
	CHECK(expected_reports.empty());
}

void ExhaustiveReporter::on_report(cero::Message message, cero::Severity, cero::SourceLocation location, std::format_args args)
{
	auto message_text = std::vformat(cero::MESSAGE_FORMATS[message], args);

	bool empty = expected_reports.empty();
	CHECK(!empty);
	if (empty)
		return;

	auto& current = expected_reports.front();
	bool  matches = current == Report {message, location, message_text};
	CHECK(matches);

	if (matches)
		expected_reports.pop();
}

void ExhaustiveReporter::on_expect(uint32_t line, uint32_t column, cero::Message message, std::format_args args)
{
	auto message_text = std::vformat(cero::MESSAGE_FORMATS[message], args);

	cero::SourceLocation location {line, column, test_name};
	expected_reports.emplace(message, location, std::vformat(cero::MESSAGE_FORMATS[message], args));
}
