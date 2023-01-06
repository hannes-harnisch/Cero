#pragma once

#include "driver/Config.hpp"
#include "driver/SourceLocation.hpp"

#include <optional>
#include <string_view>

namespace cero
{

class Source
{
	std::string		 text;
	std::string_view path;
	uint32_t		 tab_size;

public:
	using Iterator = std::string::const_iterator;

	static std::optional<Source> from_file(std::string_view path, const Config& config);

	Source(std::string source_text, std::string_view path, const Config& config);

	SourceLocation	 locate(Source::Iterator cursor) const;
	Iterator		 begin() const;
	Iterator		 end() const;
	std::string_view get_text() const;
	std::string_view get_path() const;
};

} // namespace cero