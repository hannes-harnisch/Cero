#pragma once

#include "cero/driver/Message.hpp"
#include "cero/driver/SourceLocation.hpp"

#include <format>

namespace cero
{

class Reporter
{
	bool has_error_reports	= false;
	bool warnings_as_errors = false;

public:
	virtual ~Reporter() = default;

	template<typename... Args>
	void report(CheckedMessage<Args...> message, SourceLocation location, Args&&... args)
	{
		on_report(message.value, location, std::make_format_args(std::forward<Args>(args)...));
	}

	bool has_errors() const;
	void set_warnings_as_errors(bool value);

protected:
	virtual void on_report(Message message, Severity severity, SourceLocation location, std::format_args args) = 0;

	void on_report(Message message, SourceLocation location, std::format_args args);
};

} // namespace cero
