#include "Reporter.hpp"

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
		}
		return Severity::Error;
	}
} // namespace

void Reporter::set_warnings_as_errors()
{
	warnings_as_errors = true;
}

bool Reporter::has_reports() const
{
	return !reports.empty();
}

bool Reporter::pop_report(Message message, SourceLocation location, std::format_args args)
{
	Report target(message, location, std::vformat(MESSAGE_FORMATS[message], args));

	auto it = std::find(reports.begin(), reports.end(), target);
	if (it == reports.end())
		return false;

	reports.erase(it);
	return true;
}

void Reporter::write(Message message, SourceLocation location, std::format_args args)
{
	auto severity = get_severity(message);
	if (warnings_as_errors && severity == Severity::Warning)
		severity = Severity::Error;

	auto message_text  = std::vformat(MESSAGE_FORMATS[message], args);
	auto severity_text = to_string(severity);
	std::fprintf(stderr, "%s:%u:%u: %s: %s\n", location.file.data(), location.line, location.column, severity_text,
				 message_text.data());

	reports.emplace_back(message, location, std::move(message_text));
}
