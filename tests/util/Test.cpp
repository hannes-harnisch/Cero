#include "Test.hpp"

#include <cero/driver/Build.hpp>

ExhaustiveReporter::ExhaustiveReporter(Reporter reporter, std::string_view test_name) :
	Reporter(std::move(reporter)),
	test_name(test_name)
{}

ExhaustiveReporter::~ExhaustiveReporter()
{
	CHECK(!has_reports());
}

cero::Source make_test_source(std::string source_text, std::source_location loc, const cero::Config& config)
{
	auto path = loc.function_name();
	return {std::move(source_text), path, config};
}

ExhaustiveReporter build_test_source(std::string source_text, std::source_location loc)
{
	cero::Config config;

	auto source	  = make_test_source(std::move(source_text), loc, config);
	auto reporter = build_source(source, config);
	auto path	  = loc.function_name();
	
	return {std::move(reporter), path};
}
