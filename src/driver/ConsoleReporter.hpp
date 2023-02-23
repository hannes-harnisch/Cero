#pragma once

#include "cero/driver/Config.hpp"
#include "cero/driver/Reporter.hpp"

namespace cero
{

class ConsoleReporter : public Reporter
{
public:
	explicit ConsoleReporter(const Config& config);

private:
	void on_report(Message message, Severity severity, SourceLocation location, std::format_args args) override;
};

} // namespace cero
