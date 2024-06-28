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
		begin_(it_),
		end_(source.get_text().end()) {
	}

	/// Returns the current character and then advances, or returns null if the cursor is at the end.
	std::optional<char> next() {
		if (it_ != end_) {
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
		}
	}

	/// Returns true and advances if the current character equals the expected, otherwise false.
	bool match(char expected) {
		if (it_ != end_ && *it_ == expected) {
			++it_;
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
		return static_cast<SourceOffset>(it_ - begin_);
	}

private:
	std::string_view::iterator it_;
	std::string_view::iterator begin_;
	std::string_view::iterator end_;
};

} // namespace cero
