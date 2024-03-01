#pragma once

#include "cero/io/Config.hpp"
#include "cero/io/Reporter.hpp"

namespace cero {

class ConsoleReporter : public Reporter {
public:
	explicit ConsoleReporter(const Config& config);

private:
	void handle_report(Severity severity, CodeLocation location, std::string message_text) override;
};

} // namespace cero
