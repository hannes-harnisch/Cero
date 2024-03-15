#pragma once

#include "cero/io/Source.hpp"

#include <optional>
#include <string_view>

namespace cero {

class SourceCursor {
public:
	/// Creates a cursor positioned at the start of the given source.
	explicit SourceCursor(const SourceGuard& source) :
		it_(source.get_text().begin()),
		end_(source.get_text().end()),
		offset_(0) {
	}

	struct Position {
		char character;
		SourceOffset offset;

		explicit operator bool() const {
			return offset != std::numeric_limits<SourceOffset>::max();
		}
	};

	/// Returns the current character and offset and then advances, or returns the null character and an offset with all-one
	/// bits if the cursor is at the end.
	Position next_position() {
		if (it_ != end_) {
			return {*it_++, offset_++};
		} else {
			return {'\0', std::numeric_limits<SourceOffset>::max()};
		}
	}

	/// Returns the current character and then advances, or returns null if the cursor is at the end.
	std::optional<char> next() {
		if (it_ != end_) {
			++offset_;
			return *it_++;
		} else {
			return std::nullopt;
		}
	}

	/// Returns the current character or null if the cursor is at the end.
	std::optional<char> peek() const {
		if (it_ != end_) {
			return *it_;
		} else {
			return std::nullopt;
		}
	}

	/// Moves cursor to the next character.
	void advance() {
		if (it_ != end_) {
			++it_;
			++offset_;
		}
	}

	/// Returns true and advances if the current character equals the expected, otherwise false.
	bool match(char expected) {
		if (it_ != end_ && *it_ == expected) {
			++it_;
			++offset_;
			return true;
		} else {
			return false;
		}
	}

	/// True if the cursor is not at the end.
	bool valid() const {
		return it_ != end_;
	}

	/// Current offset from the beginning of the source text.
	SourceOffset offset() const {
		return offset_;
	}

private:
	std::string_view::iterator it_;
	std::string_view::iterator end_;
	SourceOffset offset_;
};

} // namespace cero
