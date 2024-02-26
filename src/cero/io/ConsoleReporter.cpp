#include "ConsoleReporter.hpp"

namespace cero {

ConsoleReporter::ConsoleReporter(const Config& config) {
	set_warnings_as_errors(config.warnings_as_errors);
}

void ConsoleReporter::handle_report(Message, Severity severity, CodeLocation location, std::string message_text) {
	auto location_text = location.to_string();
	auto severity_text = severity_to_string(severity);
	fmt::println("{}: {}: {}", location_text, severity_text, message_text);
}

} // namespace cero
