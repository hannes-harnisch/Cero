#pragma once

#include <cero/driver/Reporter.hpp>
#include <cero/driver/Source.hpp>
#include <cero/util/Traits.hpp>
#include <doctest/doctest.h>

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public cero::Reporter,
						   public cero::Immovable
{
	std::string_view test_name;

public:
	ExhaustiveReporter(cero::Reporter reporter, std::string_view test_name);

	~ExhaustiveReporter();

	template<typename... Args>
	bool pop_report(uint32_t line, uint32_t column, cero::CheckedMessage<Args...> message, Args&&... args)
	{
		cero::SourceLocation location {line, column, test_name};
		return Reporter::pop_report(message.value, location, std::make_format_args(std::forward<Args>(args)...));
	}
};

cero::Source make_test_source(std::string		   source_text,
							  std::source_location loc	  = std::source_location::current(),
							  const cero::Config&  config = cero::Config());

ExhaustiveReporter build_test_source(std::string source_text, std::source_location loc = std::source_location::current());

#define TEST(name) DOCTEST_CREATE_AND_REGISTER_FUNCTION(Test_##name, #name)
