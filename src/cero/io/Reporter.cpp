#include "Reporter.hpp"

#include "cero/util/Fail.hpp"

namespace cero {

namespace {

	void verify_message_arg_count(Message message, size_t arg_count) {
		size_t count = 0;

		bool open = false;
		for (char c : get_message_format(message)) {
			if (c == '{') {
				open = true;
			} else if (c == '}' && open) {
				++count;
				open = false;
			}
		}

		if (count != arg_count) {
			fail_assert("Unexpected number of message arguments.");
		}
	}

} // namespace

bool Reporter::has_errors() const {
	return has_error_reports;
}

void Reporter::set_warnings_as_errors(bool value) {
	warnings_as_errors = value;
}

void Reporter::on_report(Message message, CodeLocation location, std::format_args args, size_t arg_count) {
	verify_message_arg_count(message, arg_count);

	auto severity = get_message_severity(message);
	if (warnings_as_errors && severity == Severity::Warning) {
		severity = Severity::Error;
	}

	if (severity == Severity::Error) {
		has_error_reports = true;
	}

	auto format = get_message_format(message);
	write_report(message, severity, location, std::vformat(format, args));
}

} // namespace cero
