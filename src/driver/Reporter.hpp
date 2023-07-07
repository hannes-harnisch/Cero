#pragma once

#include "driver/Message.hpp"
#include "driver/SourceLocation.hpp"

#include <format>

namespace cero {

class Reporter {
public:
	template<typename... Args>
	void report(Message message, SourceLocation location, Args&&... args) {
		on_report(message, location, std::make_format_args(std::forward<Args>(args)...), sizeof...(Args));
	}

	bool has_errors() const;
	void set_warnings_as_errors(bool value);

	virtual ~Reporter() = default;

protected:
	virtual void write_report(Message message, Severity severity, SourceLocation location, std::format_args args) = 0;

private:
	bool has_error_reports	= false;
	bool warnings_as_errors = false;

	void on_report(Message message, SourceLocation location, std::format_args args, size_t arg_count);
};

} // namespace cero
