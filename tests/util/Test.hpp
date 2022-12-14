#pragma once

#include <cero/driver/Reporter.hpp>
#include <cero/driver/Source.hpp>
#include <cero/util/Traits.hpp>
#include <doctest/doctest.h>

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public Reporter,
						   public Immovable
{
	std::string_view test_name;

public:
	ExhaustiveReporter(std::string_view test_name);
	ExhaustiveReporter(Reporter reporter, std::string_view test_name);

	~ExhaustiveReporter();

	template<typename... Args>
	bool pop_report(uint32_t line, uint32_t column, CheckedMessage<Args...> message, Args&&... args)
	{
		SourceLocation location {line, column, test_name};
		return Reporter::pop_report(message.value, location, std::make_format_args(std::forward<Args>(args)...));
	}
};

Source make_test_source(std::string			 source_text,
						std::source_location loc	= std::source_location::current(),
						const Config&		 config = Config());

ExhaustiveReporter build_test_source(std::string source_text, std::source_location loc = std::source_location::current());

#define TEST(name) DOCTEST_CREATE_AND_REGISTER_FUNCTION(Test_##name, #name)
