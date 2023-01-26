#pragma once

#include <cero/driver/Reporter.hpp>
#include <cero/util/Traits.hpp>

#include <format>
#include <queue>
#include <source_location>
#include <string>

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public cero::Reporter,
						   public cero::Immovable
{
	struct Report
	{
		cero::Message		 message;
		cero::SourceLocation location;
		std::string			 text;

		bool operator==(const Report&) const = default;
	};

	std::queue<Report> expected_reports;
	std::string_view   test_name;

public:
	explicit ExhaustiveReporter(std::source_location location = std::source_location::current());

	~ExhaustiveReporter() override;

	template<typename... Args>
	void expect(uint32_t line, uint32_t column, cero::CheckedMessage<Args...> message, Args&&... args)
	{
		on_expect(line, column, message.value, std::make_format_args(std::forward<Args>(args)...));
	}

	bool has_errors() const override;

private:
	void on_report(cero::Message message, cero::SourceLocation location, std::format_args args) override;
	void on_expect(uint32_t line, uint32_t column, cero::Message message, std::format_args args);
};