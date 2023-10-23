#pragma once

#include "driver/Source.hpp"

#include <optional>
#include <string_view>

namespace cero {

class LexCursor {
public:
	explicit LexCursor(const Source& source);

	// Returns the current character or null if the cursor is at the end.
	std::optional<char> peek() const;

	// Returns the current character and then advances, or returns null if the cursor is at the end.
	std::optional<char> next();

	// Moves cursor to the next character.
	void advance();

	// Returns true and advances if the current character equals the expected, otherwise false.
	bool match(char expected);

	// True if the cursor is not at the end.
	bool valid() const;

	// Returns a view of the remaining source text after the cursor position.
	std::string_view rest() const;

	// Returns how many characters are left in the source text after the cursor position.
	uint32_t count_rest() const;

private:
	std::string_view::iterator current;
	std::string_view::iterator end;
};

} // namespace cero