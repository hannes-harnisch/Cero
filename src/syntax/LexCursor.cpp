#include "LexCursor.hpp"

namespace cero {

LexCursor::LexCursor(std::string_view source_text) :
	current(source_text.begin()),
	end(source_text.end()) {
}

std::optional<char> LexCursor::peek() const {
	if (current != end) {
		return *current;
	} else {
		return std::nullopt;
	}
}

std::optional<char> LexCursor::next() {
	if (current != end) {
		return *current++;
	} else {
		return std::nullopt;
	}
}

void LexCursor::advance() {
	if (current != end) {
		++current;
	}
}

bool LexCursor::match(char expected) {
	if (current == end) {
		return false;
	}

	if (*current != expected) {
		return false;
	}

	++current;
	return true;
}

bool LexCursor::valid() const {
	return current != end;
}

std::string_view LexCursor::rest() const {
	return std::string_view(current, end);
}

uint32_t LexCursor::count_rest() const {
	return static_cast<uint32_t>(end - current);
}

} // namespace cero