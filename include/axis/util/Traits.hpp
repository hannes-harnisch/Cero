#pragma once

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

} // namespace cero