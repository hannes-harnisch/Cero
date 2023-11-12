#pragma once

#include "cero/io/CodeLocation.hpp"
#include "cero/io/Message.hpp"

#include <format>

namespace cero {

class Reporter {
public:
	template<typename... Args>
	void report(Message message, CodeLocation location, Args&&... args) {
		on_report(message, location, std::make_format_args(std::forward<Args>(args)...), sizeof...(Args));
	}

	bool has_errors() const;
	void set_warnings_as_errors(bool value);

	virtual ~Reporter() = default;

protected:
	virtual void write_report(Message message, Severity severity, CodeLocation location, std::string message_text) = 0;

private:
	bool has_error_reports = false;
	bool warnings_as_errors = false;

	void on_report(Message message, CodeLocation location, std::format_args args, size_t arg_count);
};

} // namespace cero
