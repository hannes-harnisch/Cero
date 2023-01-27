#pragma once

#include "cero/util/Traits.hpp"
#include "util/Macros.hpp"

namespace cero
{

template<typename D>
class Defer : public Immovable
{
	D deferred;

public:
	Defer(D&& deferred) :
		deferred(std::move(deferred))
	{}

	~Defer()
	{
		deferred();
	}
};

} // namespace cero

#define defer cero::Defer CERO_CONCAT(_defer_, __LINE__) = [&]() -> void
