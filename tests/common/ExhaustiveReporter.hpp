#pragma once

#include <cero/io/CodeLocation.hpp>
#include <cero/io/Message.hpp>
#include <cero/io/Reporter.hpp>

#include <queue>
#include <string>
#include <string_view>

namespace tests {

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public cero::Reporter {
public:
	ExhaustiveReporter();
	~ExhaustiveReporter() override;

	void expect(uint32_t line, uint32_t column, cero::Message message, cero::ReportArgs args);

	ExhaustiveReporter(ExhaustiveReporter&&) = delete;
	ExhaustiveReporter& operator=(ExhaustiveReporter&&) = delete;

private:
	struct Report {
		cero::CodeLocation location;
		std::string message_text;

		bool operator==(const Report&) const = default;
	};

	std::queue<Report> expected_reports_;
	std::string_view test_name_;

	void handle_report(cero::Severity severity, cero::CodeLocation location, std::string message_text) override;
};

} // namespace tests
