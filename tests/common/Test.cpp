#include "Test.hpp"

#include <cero/driver/BuildCommand.hpp>

const char* get_current_test_name();

cero::SourceLock make_test_source(std::string_view source_text, const cero::Config& config) {
	std::string_view path = get_current_test_name();
	return cero::Source::from_text(path, source_text, config).lock().value();
}

void build_test_source(cero::Reporter& reporter, std::string_view source_text) {
	cero::Config config;
	std::string_view path = get_current_test_name();
	auto source = cero::Source::from_text(path, source_text, config);
	cero::build_source(source, config, reporter);
}
