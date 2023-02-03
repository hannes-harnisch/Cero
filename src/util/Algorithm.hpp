#pragma once

#include <algorithm>

namespace cero
{

bool contains(const auto& range, const auto& value)
{
	return std::find(std::begin(range), std::end(range), value) != std::end(range);
}

} // namespace cero
