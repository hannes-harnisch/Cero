#pragma once

#include "util/Enum.hpp"

template<typename Key, typename Value, size_t COUNT = size_from_enum_max<Key>()>
class LookupTable
{
public:
	constexpr Value& operator[](Key key)
	{
		return table[static_cast<size_t>(key)];
	}

	constexpr Value operator[](Key key) const
	{
		return table[static_cast<size_t>(key)];
	}

private:
	Value table[COUNT] = {};
};
