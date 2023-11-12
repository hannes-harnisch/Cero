#include "ExhaustiveReporter.hpp"

#include <doctest/doctest.h>

const char* get_current_test_name();

ExhaustiveReporter::ExhaustiveReporter() :
	test_name_(get_current_test_name()) {
}

ExhaustiveReporter::~ExhaustiveReporter() {
	CHECK(expected_reports_.empty());
}

void ExhaustiveReporter::write_report(cero::Message message,
									  cero::Severity,
									  cero::CodeLocation location,
									  std::string message_text) {
	REQUIRE(!expected_reports_.empty());

	const Report& expected = expected_reports_.front();
	const Report received {message, location, std::move(message_text)};
	const bool matches = expected == received;
	CHECK(matches);

	if (matches) {
		expected_reports_.pop();
	}
}

void ExhaustiveReporter::on_expect(uint32_t line, uint32_t column, cero::Message message, std::format_args args) {
	auto format = cero::get_message_format(message);
	auto message_text = std::vformat(format, args);

	cero::CodeLocation location {test_name_, line, column};
	expected_reports_.emplace(message, location, std::move(message_text));
}
