#pragma once

#include <cstdint>
#include <string_view>

namespace cero {

struct SourceLocation {
	std::string_view file;
	uint32_t		 line	= 0;
	uint32_t		 column = 0;

	std::string to_string() const;

	bool operator==(const SourceLocation&) const = default;
};

} // namespace cero
