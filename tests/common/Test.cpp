#include "Test.hpp"

#include <cero/driver/BuildCommand.hpp>

namespace tests {

cero::SourceGuard make_test_source(std::string_view source_text, const cero::Configuration& config) {
	std::string_view name = get_current_test_name();
	return cero::Source::from_string(name, source_text, config).lock().or_throw();
}

void build_test_source(cero::Reporter& reporter, std::string_view source_text) {
	cero::Configuration config;
	std::string_view name = get_current_test_name();
	auto source = cero::Source::from_string(name, source_text, config);
	cero::build_source(source, config, reporter);
}

} // namespace tests
