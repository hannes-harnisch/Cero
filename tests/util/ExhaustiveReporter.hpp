#pragma once

#include <cero/driver/Reporter.hpp>
#include <cero/driver/Source.hpp>
#include <cero/util/Traits.hpp>

// A test utility that triggers a test failure if there are unexpected messages left after running the compiler.
class ExhaustiveReporter : public Reporter,
						   public Immovable
{
public:
	static ExhaustiveReporter from(std::string_view file_path);

	ExhaustiveReporter() = default;
	ExhaustiveReporter(std::string source_text, std::source_location loc = std::source_location::current());

	~ExhaustiveReporter();

private:
	ExhaustiveReporter(const Source& source);
};

const char* test_name(std::source_location loc = std::source_location::current());
