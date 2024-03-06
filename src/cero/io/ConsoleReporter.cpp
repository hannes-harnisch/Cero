#include "ConsoleReporter.hpp"

namespace cero {

ConsoleReporter::ConsoleReporter(const Configuration& config) {
	set_warnings_as_errors(config.warnings_as_errors);
}

void ConsoleReporter::handle_report(MessageLevel message_level, CodeLocation location, std::string message_text) {
	auto location_str = location.to_string();
	auto msg_level_str = message_level_to_string(message_level);
	fmt::println("{}: {}: {}", location_str, msg_level_str, message_text);
}

} // namespace cero
