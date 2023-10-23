#pragma once

#include "driver/Config.hpp"
#include "driver/Reporter.hpp"

namespace cero {

class ConsoleReporter : public Reporter {
public:
	explicit ConsoleReporter(const Config& config);

private:
	void write_report(Message message, Severity severity, SourceLocation location, std::string message_text) override;
};

} // namespace cero
