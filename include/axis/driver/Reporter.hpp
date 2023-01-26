#pragma once

#include "cero/driver/Message.hpp"
#include "cero/driver/SourceLocation.hpp"

#include <format>

namespace cero
{

class Reporter
{
public:
	virtual ~Reporter() = default;

	template<typename... Args>
	void report(CheckedMessage<Args...> message, SourceLocation location, Args&&... args)
	{
		on_report(message.value, location, std::make_format_args(std::forward<Args>(args)...));
	}

	virtual bool has_errors() const = 0;

protected:
	virtual void on_report(Message message, SourceLocation location, std::format_args args) = 0;
};

} // namespace cero
