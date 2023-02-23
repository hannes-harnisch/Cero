#include "cero/driver/Reporter.hpp"

namespace cero
{

namespace
{
	Severity get_severity(Message message)
	{
		using enum Message;
		switch (message)
		{
			case UnnecessaryColonBeforeBlock: return Severity::Warning;
			default: return Severity::Error;
		}
	}
} // namespace

bool Reporter::has_errors() const
{
	return has_error_reports;
}

void Reporter::set_warnings_as_errors(bool value)
{
	warnings_as_errors = value;
}

void Reporter::on_report(Message message, SourceLocation location, std::format_args args)
{
	auto severity = get_severity(message);
	if (warnings_as_errors && severity == Severity::Warning)
		severity = Severity::Error;

	if (severity == Severity::Error)
		has_error_reports = true;

	on_report(message, severity, location, args);
}

} // namespace cero
