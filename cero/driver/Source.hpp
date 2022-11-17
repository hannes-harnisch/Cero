#pragma once

#include "driver/Config.hpp"
#include "driver/SourceLocation.hpp"

#include <optional>
#include <string_view>

class Source
{
	std::string		 text;
	std::string_view path;
	uint32_t		 tab_size;

public:
	using Iterator = std::string::const_iterator;

	static std::optional<Source> from_file(std::string_view path, const Config& config);

	// primarily intended for testing
	Source(std::string source_text, std::string_view path_for_messages, uint32_t tab_size = Config::DEFAULT_TAB_SIZE);

	SourceLocation	 locate(Source::Iterator cursor) const;
	Iterator		 begin() const;
	Iterator		 end() const;
	std::string_view get_text() const;
	std::string_view get_path() const;
};
