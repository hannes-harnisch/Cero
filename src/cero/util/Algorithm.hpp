#pragma once

#include <algorithm>
#include <ranges>

namespace cero {

template<typename Range>
bool contains(const Range& range, const std::ranges::range_value_t<Range>& value) {
	auto end = std::end(range);
	return std::find(std::begin(range), end, value) != end;
}

} // namespace cero
