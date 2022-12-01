#pragma once

#include "util/Macros.hpp"
#include "util/Traits.hpp"

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

#define defer Defer CERO_CONCAT(_defer_, __LINE__) = [&]()
