#pragma once

#include "cero/driver/Config.hpp"
#include "cero/driver/Reporter.hpp"

#include <format>
#include <string>
#include <vector>

namespace cero
{

class ConsoleReporter : public Reporter
{
	bool has_error_reports	= false;
	bool warnings_as_errors = false;

public:
	explicit ConsoleReporter(const Config& config);

	bool has_errors() const override;

private:
	void on_report(Message message, SourceLocation location, std::format_args args) override;
};

} // namespace cero
