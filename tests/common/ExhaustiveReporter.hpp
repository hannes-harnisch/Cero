#pragma once

#include <driver/Reporter.hpp>

#include <format>
#include <queue>
#include <source_location>
#include <string>

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public cero::Reporter {
	struct Report {
		cero::Message message;
		cero::SourceLocation location;
		std::string text;

		bool operator==(const Report&) const = default;
	};

	std::queue<Report> expected_reports;
	std::string_view test_name;

public:
	explicit ExhaustiveReporter(std::source_location location = std::source_location::current());

	~ExhaustiveReporter() override;

	template<typename... Args>
	void expect(uint32_t line, uint32_t column, cero::Message message, Args&&... args) {
		on_expect(line, column, message, std::make_format_args(std::forward<Args>(args)...));
	}

	ExhaustiveReporter(const ExhaustiveReporter&) = delete;
	ExhaustiveReporter& operator=(const ExhaustiveReporter&) = delete;

private:
	void write_report(cero::Message message,
					  cero::Severity severity,
					  cero::SourceLocation location,
					  std::format_args args) override;

	void on_expect(uint32_t line, uint32_t column, cero::Message message, std::format_args args);
};
