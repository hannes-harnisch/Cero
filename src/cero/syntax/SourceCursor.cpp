#include "SourceCursor.hpp"

namespace cero {

SourceCursor::SourceCursor(const SourceLock& source) :
	it_(source.get_text().begin()),
	end_(source.get_text().end()),
	offset_(0) {
}

std::optional<char> SourceCursor::next() {
	if (it_ != end_) {
		++offset_;
		return *it_++;
	} else {
		return std::nullopt;
	}
}

std::optional<char> SourceCursor::peek() const {
	if (it_ != end_) {
		return *it_;
	} else {
		return std::nullopt;
	}
}

void SourceCursor::advance() {
	if (it_ != end_) {
		++it_;
		++offset_;
	}
}

bool SourceCursor::match(char expected) {
	if (it_ != end_ && *it_ == expected) {
		++it_;
		++offset_;
		return true;
	} else {
		return false;
	}
}

bool SourceCursor::valid() const {
	return it_ != end_;
}

uint32_t SourceCursor::offset() const {
	return offset_;
}

} // namespace cero
