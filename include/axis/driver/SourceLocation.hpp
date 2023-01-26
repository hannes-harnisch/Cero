#pragma once

#include <cstdint>
#include <string_view>

namespace cero
{

struct SourceLocation
{
	uint32_t		 line	= 0;
	uint32_t		 column = 0;
	std::string_view file;

	bool operator==(const SourceLocation&) const = default;

	std::string to_string() const;
};

} // namespace cero