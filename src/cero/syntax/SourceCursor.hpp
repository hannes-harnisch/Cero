#pragma once

#include "cero/io/Source.hpp"

#include <optional>
#include <string_view>

namespace cero {

class SourceCursor {
public:
	explicit SourceCursor(const SourceLock& source);

	// Returns the current character and then advances, or returns null if the cursor is at the end.
	std::optional<char> next();

	// Returns the current character or null if the cursor is at the end.
	std::optional<char> peek() const;

	// Moves cursor to the next character.
	void advance();

	// Returns true and advances if the current character equals the expected, otherwise false.
	bool match(char expected);

	// True if the cursor is not at the end.
	bool valid() const;

	// Current offset from the beginning of the source text.
	uint32_t offset() const;

private:
	std::string_view::iterator it_;
	std::string_view::iterator end_;
	uint32_t offset_;
};

} // namespace cero