#include "SourceCursor.hpp"

namespace cero {

SourceCursor::SourceCursor(const SourceGuard& source) :
	it_(source.get_text().begin()),
	end_(source.get_text().end()),
	offset_(0) {
}

SourceCursor::Position SourceCursor::next_position() {
	if (it_ != end_) {
		return {*it_++, offset_++};
	} else {
		return {'\0', UINT32_MAX};
	}
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

SourceOffset SourceCursor::offset() const {
	return offset_;
}

SourceOffset SourceCursor::remaining_length() const {
	return static_cast<SourceOffset>(end_ - it_);
}

} // namespace cero
