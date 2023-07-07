#include "Reporter.hpp"

namespace cero {

bool Reporter::has_errors() const {
	return has_error_reports;
}

void Reporter::set_warnings_as_errors(bool value) {
	warnings_as_errors = value;
}

void Reporter::on_report(Message message, SourceLocation location, std::format_args args, size_t arg_count) {
	verify_message_arg_count(message, arg_count);

	auto severity = get_message_severity(message);
	if (warnings_as_errors && severity == Severity::Warning)
		severity = Severity::Error;

	if (severity == Severity::Error)
		has_error_reports = true;

	write_report(message, severity, location, args);
}

} // namespace cero
