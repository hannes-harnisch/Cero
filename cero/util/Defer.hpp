#pragma once

#include "util/Macros.hpp"
#include "util/Traits.hpp"

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

#define defer cero::Defer CERO_CONCAT(_defer_, __LINE__) = [&]()
