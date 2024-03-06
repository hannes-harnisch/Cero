#include "ExhaustiveReporter.hpp"

#include "common/Test.hpp"

#include <doctest/doctest.h>

namespace tests {

ExhaustiveReporter::ExhaustiveReporter() :
	test_name_(get_current_test_name()) {
}

ExhaustiveReporter::~ExhaustiveReporter() {
	// If this fails, not all expected reports were seen.
	CHECK(expected_reports_.empty());
}

void ExhaustiveReporter::expect(uint32_t line, uint32_t column, cero::Message message, cero::MessageArgs args) {
	auto format = cero::get_message_format(message);
	auto message_text = fmt::vformat(format, args.store);

	cero::CodeLocation location {test_name_, line, column};
	expected_reports_.emplace(location, std::move(message_text));
}

void ExhaustiveReporter::handle_report(cero::MessageLevel, cero::CodeLocation location, std::string message_text) {
	const bool reports_not_exhausted = !expected_reports_.empty();
	REQUIRE(reports_not_exhausted); // If this fails, every expected report was already seen and an unexpected one was received.

	const Report& expected = expected_reports_.front();
	const Report received {location, std::move(message_text)};
	const bool report_matches = expected == received;
	CHECK(report_matches); // If this fails, the received report does not match the report that is expected to be seen next.

	if (report_matches) {
		expected_reports_.pop();
	}
}

} // namespace tests
