#pragma once

#include "cero/io/CodeLocation.hpp"
#include "cero/io/Message.hpp"

namespace cero {

struct ReportArgs {
	fmt::dynamic_format_arg_store<fmt::format_context> store;
	const size_t count = 0;

	ReportArgs() = default;

	template<typename... Args>
	explicit ReportArgs(Args&&... args) :
		count(sizeof...(Args)) {
		store_args(fmt::make_format_args(args...));
	}

private:
	void store_args(fmt::format_args args);
};

class Reporter {
public:
	virtual ~Reporter() = default;

	void report(Message message, CodeLocation location, ReportArgs args);

	bool has_errors() const;
	void set_warnings_as_errors(bool value);

private:
	bool has_error_reports_ = false;
	bool warnings_as_errors_ = false;

	virtual void handle_report(Message message, Severity severity, CodeLocation location, std::string message_text) = 0;
};

} // namespace cero
