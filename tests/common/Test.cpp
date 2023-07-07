#include "Test.hpp"

#include <driver/BuildCommand.hpp>
#include <util/Traits.hpp>

cero::Source make_test_source(std::string_view source_text, std::source_location location, const cero::Config& config) {
	auto path = cero::normalize_function_name(location);
	return cero::Source::from_text(source_text, path, config);
}

void build_test_source(cero::Reporter& reporter, std::string_view source_text, std::source_location location) {
	cero::Config config;

	auto source = make_test_source(source_text, location, config);
	cero::build_source(source, config, reporter);
}
