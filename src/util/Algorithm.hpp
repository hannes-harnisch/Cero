#pragma once

#include <algorithm>

namespace cero
{

bool contains(const auto& range, const auto& value)
{
	return std::find(std::begin(range), std::end(range), value) != std::end(range);
}

template<typename... Types, typename... Alternatives>
bool holds_any_of(const std::variant<Alternatives...>& variant)
{
	return (... || std::holds_alternative<Types>(variant));
}

} // namespace cero
