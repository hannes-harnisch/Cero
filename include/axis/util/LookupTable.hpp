#pragma once

#include <magic_enum.hpp>

#include <array>

namespace cero
{

template<typename Key, typename Value>
class LookupTable
{
	template<typename T>
	static constexpr size_t size_from_enum_max()
	requires std::is_enum_v<T>
	{
		auto values = magic_enum::enum_values<T>();
		auto max	= std::max_element(values.begin(), values.end());
		return static_cast<size_t>(*max) + 1;
	}

	std::array<Value, size_from_enum_max<Key>()> table;

public:
	constexpr LookupTable(Value default_value = {})
	{
		for (auto& value : table)
			value = default_value;
	}

	constexpr Value& operator[](Key key)
	{
		return table.at(static_cast<size_t>(key));
	}

	constexpr Value operator[](Key key) const
	{
		return table.at(static_cast<size_t>(key));
	}

	constexpr size_t size() const
	{
		return table.size();
	}
};

} // namespace cero
