#include "ConsoleReporter.hpp"

namespace cero {

ConsoleReporter::ConsoleReporter(const Config& config) {
	set_warnings_as_errors(config.warnings_as_errors);
}

void ConsoleReporter::write_report(Message message, Severity severity, SourceLocation location, std::format_args args) {
	auto location_text = location.to_string();
	auto severity_text = to_string(severity);
	auto message_text = std::vformat(get_message_format(message), args);
	std::cout << std::format("{}: {}: {}\n", location_text, severity_text, message_text);
}

} // namespace cero
