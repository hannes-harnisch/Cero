#pragma once

#include "driver/Message.hpp"
#include "driver/SourceLocation.hpp"

#include <format>
#include <string_view>
#include <vector>

class Reporter
{
	struct Report
	{
		Message		   message;
		SourceLocation location;
		std::string	   text;

		bool operator==(const Report&) const = default;
	};

	std::vector<Report> reports;
	bool				warnings_as_errors = false;

public:
	template<Message MESSAGE, typename... Args>
	void report(SourceLocation location, Args&&... args)
	{
		write(MESSAGE, location, get_format(MESSAGE), std::make_format_args(std::forward<Args>(args)...));
	}

	// For testing purposes
	template<typename... Args>
	bool pop_report(Message message, SourceLocation location, Args&&... args)
	{
		return do_pop_report(message, location, std::make_format_args(std::forward<Args>(args)...));
	}

	void set_warnings_as_errors();
	bool has_reports() const;

private:
	void write(Message message, SourceLocation loc, std::string_view format, std::format_args args);
	bool do_pop_report(Message message, SourceLocation location, std::format_args args);
};
