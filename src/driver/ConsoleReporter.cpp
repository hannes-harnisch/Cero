#include "ConsoleReporter.hpp"

namespace cero
{

namespace
{
	enum class Severity
	{
		Error,
		Warning,
		Note,
	};

	const char* to_string(Severity severity)
	{
		switch (severity)
		{
			case Severity::Error: return "error";
			case Severity::Warning: return "warning";
			case Severity::Note: return "note";
		}
		return nullptr;
	}

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

ConsoleReporter::ConsoleReporter(const Config& config)
{
	if (config.warnings_as_errors)
		warnings_as_errors = true;
}

bool ConsoleReporter::has_errors() const
{
	return has_error_reports;
}

void ConsoleReporter::on_report(Message message, SourceLocation location, std::format_args args)
{
	auto severity = get_severity(message);
	if (warnings_as_errors && severity == Severity::Warning)
		severity = Severity::Error;

	if (severity == Severity::Error)
		has_error_reports = true;

	auto location_text = location.to_string();
	auto severity_text = to_string(severity);
	auto message_text  = std::vformat(MESSAGE_FORMATS[message], args);
	std::cout << std::format("{}: {}: {}\n", location_text, severity_text, message_text);
}

} // namespace cero
