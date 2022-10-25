#pragma once

#include "driver/Reporter.hpp"

#include <optional>
#include <string_view>

class Source
{
	std::string		 text;
	std::string_view path;

public:
	using Iterator = std::string::const_iterator;

	static std::optional<Source> from(std::string_view path);

	Source(std::string text);

	SourceLocation	 locate(Source::Iterator cursor) const;
	Iterator		 begin() const;
	Iterator		 end() const;
	std::string_view get_text() const;
	std::string_view get_path() const;

private:
	Source(std::string text, std::string_view path);
};
