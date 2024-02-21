#pragma once

#include "cero/io/CodeLocation.hpp"
#include "cero/io/Message.hpp"

namespace cero {

class Reporter {
public:
	template<typename... Args>
	void report(Message message, CodeLocation location, Args&&... args) {
		on_report(message, location, fmt::make_format_args(args...), sizeof...(Args));
	}

	bool has_errors() const;
	void set_warnings_as_errors(bool value);

	virtual ~Reporter() = default;

private:
	bool has_error_reports_ = false;
	bool warnings_as_errors_ = false;

	virtual void handle_report(Message message, Severity severity, CodeLocation location, std::string message_text) = 0;

	void on_report(Message message, CodeLocation location, fmt::format_args args, size_t arg_count);
};

} // namespace cero
