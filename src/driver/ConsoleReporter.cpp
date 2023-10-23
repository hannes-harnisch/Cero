#include "ConsoleReporter.hpp"

namespace cero {

ConsoleReporter::ConsoleReporter(const Config& config) {
	set_warnings_as_errors(config.warnings_as_errors);
}

void ConsoleReporter::write_report(Message, Severity severity, SourceLocation location, std::string message_text) {
	auto location_text = location.to_string();
	auto severity_text = to_string(severity);
	std::cout << std::format("{}: {}: {}\n", location_text, severity_text, message_text);
}

} // namespace cero
