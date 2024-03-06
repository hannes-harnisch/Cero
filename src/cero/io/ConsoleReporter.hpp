#pragma once

#include "cero/io/Configuration.hpp"
#include "cero/io/Reporter.hpp"

namespace cero {

/// A reporter implementation that prints to the standard output or standard error streams.
class ConsoleReporter : public Reporter {
public:
	explicit ConsoleReporter(const Configuration& config);

private:
	void handle_report(MessageLevel message_level, CodeLocation location, std::string message_text) override;
};

} // namespace cero
