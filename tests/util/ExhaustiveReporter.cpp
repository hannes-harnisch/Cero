#include "ExhaustiveReporter.hpp"

#include "util/TestSource.hpp"

#include <cero/driver/Driver.hpp>
#include <doctest/doctest.h>

ExhaustiveReporter ExhaustiveReporter::from(std::string_view file_path)
{
	return ExhaustiveReporter(Source::from_file(file_path, Config()).value());
}

ExhaustiveReporter::ExhaustiveReporter(std::string source_text, std::source_location loc) :
	ExhaustiveReporter(TestSource(std::move(source_text), loc))
{}

ExhaustiveReporter::~ExhaustiveReporter()
{
	CHECK(!has_reports());
}

ExhaustiveReporter::ExhaustiveReporter(const Source& source) :
	Reporter(build_file(source))
{}

const char* test_name(std::source_location loc)
{
	return loc.function_name();
}
