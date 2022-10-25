#pragma once

#include "driver/Message.hpp"
#include "syntax/TokenStream.hpp"

#include <format>
#include <string_view>
#include <vector>

struct SourceLocation
{
	uint32_t		 line	= 0;
	uint32_t		 column = 0;
	std::string_view file;

	bool operator==(const SourceLocation&) const = default;
};

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
	TokenStream			token_stream;
	bool				warnings_as_errors = false;

public:
	template<Message MESSAGE, typename... Args>
	void report(SourceLocation location, Args&&... args)
	{
		static_assert(arg_count_matches_message<Args...>(MESSAGE),
					  "The given number of arguments does not match required number of arguments for this message.");

		write(MESSAGE, location, get_format(MESSAGE), std::make_format_args(std::forward<Args>(args)...));
	}

	// For testing purposes
	template<typename... Args>
	bool pop_report(Message message, SourceLocation location, Args&&... args)
	{
		return do_pop_report(message, location, std::make_format_args(std::forward<Args>(args)...));
	}

	void			   set_warnings_as_errors();
	bool			   has_reports() const;
	const TokenStream& get_token_stream() const;
	void			   finalize(TokenStream tokens);

private:
	// Determines whether the number of parameters in the parameter pack matches the number of placeholders in the format string
	// associated with the message.
	template<typename... Ts>
	static consteval bool arg_count_matches_message(Message message)
	{
		size_t count = 0;

		auto format = get_format(message).data();
		while (*format)
			if (*format++ == '{')
				if (*format++ == '}')
					++count;

		return count == sizeof...(Ts);
	}

	void write(Message message, SourceLocation loc, std::string_view format, std::format_args args);
	bool do_pop_report(Message message, SourceLocation location, std::format_args args);
};
