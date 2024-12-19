#pragma once

#include "cero/io/CodeLocation.hpp"
#include "cero/io/Message.hpp"

namespace cero {

/// Stores formatting arguments for diagnostic messages.
struct MessageArgs {
	fmt::dynamic_format_arg_store<fmt::format_context> store;

	MessageArgs() = default;

	explicit MessageArgs(auto&&... args) {
		store_args(fmt::make_format_args(args...));
	}

	/// Checks if the number of message arguments matches the expected number of arguments for that message.
	bool verify_message_arg_count(Message message) const;

private:
	void store_args(fmt::format_args args);
};

/// Abstract base for implementing different ways to report diagnostics.
class Reporter {
public:
	virtual ~Reporter() = default;

	/// Reports a diagnostic message. If the number of message arguments does not match the expected number for the given
	/// message, the compiler will terminate itself. This runtime check is preferred over a compile-time check to reduce
	/// template instantiations and because every place a message can be emitted should have a unit test anyway.
	void report(Message message, CodeLocation location, MessageArgs args);

	/// Whether the reporter has encountered any error reports.
	bool has_errors() const;

	/// Whether the reporter should consider warning reports as errors.
	void set_warnings_as_errors(bool value);

private:
	bool has_error_reports_ = false;
	bool warnings_as_errors_ = false;

	/// Will be called by the report method. Override to handle how the report is actually emitted.
	virtual void handle_report(MessageLevel message_level, CodeLocation location, std::string message_text) = 0;
};

} // namespace cero
