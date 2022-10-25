#pragma once

#include <string_view>
#include <unordered_map>

struct StringHash
{
	using is_transparent = void;

	size_t operator()(std::string_view txt) const
	{
		return std::hash<std::string_view>()(txt);
	}
};

template<typename V>
using StringMap = std::unordered_map<std::string, V, StringHash, std::equal_to<>>;
