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
	template<typename... Args>
	void report(CheckedMessage<Args...> message, SourceLocation location, Args&&... args)
	{
		write(message.value, location, std::make_format_args(std::forward<Args>(args)...));
	}

	// For testing purposes
	template<typename... Args>
	bool pop_report(CheckedMessage<Args...> message, SourceLocation location, Args&&... args)
	{
		return do_pop_report(message.value, location, std::make_format_args(std::forward<Args>(args)...));
	}

	void set_warnings_as_errors();
	bool has_reports() const;

private:
	void write(Message message, SourceLocation loc, std::format_args args);
	bool do_pop_report(Message message, SourceLocation location, std::format_args args);
};
