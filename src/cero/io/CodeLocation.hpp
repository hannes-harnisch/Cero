#pragma once

#include <cstdint>
#include <string_view>

namespace cero {

/// A location in Cero source code.
struct CodeLocation {
	std::string_view source_name;
	uint32_t line = 0;
	uint32_t column = 0;

	/// Create a string representation for diagnostic messages.
	std::string to_string() const;

	/// Create a string containing only the line and column, such as for AST dumps.
	std::string to_short_string() const;

	bool operator==(const CodeLocation&) const = default;
};

} // namespace cero
