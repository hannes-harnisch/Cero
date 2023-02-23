#include "ConsoleReporter.hpp"

namespace cero
{

namespace
{
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
} // namespace

ConsoleReporter::ConsoleReporter(const Config& config)
{
	set_warnings_as_errors(config.warnings_as_errors);
}

void ConsoleReporter::on_report(Message message, Severity severity, SourceLocation location, std::format_args args)
{
	auto location_text = location.to_string();
	auto severity_text = to_string(severity);
	auto message_text  = std::vformat(MESSAGE_FORMATS[message], args);
	std::cout << std::format("{}: {}: {}\n", location_text, severity_text, message_text);
}

} // namespace cero
