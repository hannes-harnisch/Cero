#include "Test.hpp"

#include <cero/driver/Build.hpp>
#include <cero/util/Traits.hpp>

cero::Source make_test_source(std::string source_text, std::source_location location, const cero::Config& config)
{
	auto path = cero::normalize_function_name(location);
	return {std::move(source_text), path, config};
}

void build_test_source(cero::Reporter& reporter, std::string source_text, std::source_location location)
{
	cero::Config config;

	auto source = make_test_source(std::move(source_text), location, config);
	cero::build_source(source, config, reporter);
}
