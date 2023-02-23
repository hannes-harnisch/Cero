#pragma once

#include <source_location>
#include <string_view>

namespace cero
{

// Helper base class for making a class immovable and uncopyable.
class Immovable
{
public:
	Immovable() = default;

	Immovable(const Immovable&)			   = delete;
	Immovable& operator=(const Immovable&) = delete;

	Immovable(Immovable&&)			  = delete;
	Immovable& operator=(Immovable&&) = delete;
};

std::string_view normalize_function_name(std::source_location location);

} // namespace cero
