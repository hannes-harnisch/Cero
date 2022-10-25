#pragma once

#include <magic_enum.hpp>

template<typename T>
constexpr size_t size_from_enum_max()
requires std::is_enum_v<T>
{
	auto values = magic_enum::enum_values<T>();
	auto max	= std::max_element(values.begin(), values.end());
	return static_cast<size_t>(*max) + 1;
}
