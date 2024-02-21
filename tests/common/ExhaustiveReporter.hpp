#pragma once

#include <cero/io/CodeLocation.hpp>
#include <cero/io/Message.hpp>
#include <cero/io/Reporter.hpp>

#include <queue>
#include <string>
#include <string_view>

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public cero::Reporter {
public:
	ExhaustiveReporter();

	~ExhaustiveReporter() override;

	template<typename... Args>
	void expect(uint32_t line, uint32_t column, cero::Message message, Args&&... args) {
		on_expect(line, column, message, fmt::make_format_args(args...));
	}

	ExhaustiveReporter(const ExhaustiveReporter&) = delete;
	ExhaustiveReporter& operator=(const ExhaustiveReporter&) = delete;

private:
	struct Report {
		cero::Message message;
		cero::CodeLocation location;
		std::string text;

		bool operator==(const Report&) const = default;
	};

	std::queue<Report> expected_reports_;
	std::string_view test_name_;

	void handle_report(cero::Message message,
					   cero::Severity severity,
					   cero::CodeLocation location,
					   std::string message_text) override;

	void on_expect(uint32_t line, uint32_t column, cero::Message message, fmt::format_args args);
};
