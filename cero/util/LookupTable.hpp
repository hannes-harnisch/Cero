#pragma once

#include <magic_enum.hpp>

namespace cero
{

template<typename T>
constexpr size_t size_from_enum_max()
requires std::is_enum_v<T>
{
	auto values = magic_enum::enum_values<T>();
	auto max	= std::max_element(values.begin(), values.end());
	return static_cast<size_t>(*max) + 1;
}

template<typename Key, typename Value>
class LookupTable
{
	Value table[size_from_enum_max<Key>()];

public:
	static constexpr size_t COUNT = std::extent_v<decltype(table)>;

	constexpr LookupTable(Value default_value)
	{
		for (auto& value : table)
			value = default_value;
	}

	constexpr Value& operator[](Key key)
	{
		return table[static_cast<size_t>(key)];
	}

	constexpr Value operator[](Key key) const
	{
		return table[static_cast<size_t>(key)];
	}
};

} // namespace cero