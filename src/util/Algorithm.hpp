#pragma once

#include <algorithm>

namespace cero {

template<typename Range, typename T>
bool contains(const Range& range, const T& value) {
	auto end = std::end(range);
	return std::find(std::begin(range), end, value) != end;
}

} // namespace cero
