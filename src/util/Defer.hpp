#pragma once

#include "util/Macros.hpp"

namespace cero {

template<typename D>
class Defer {
	D deferred;

public:
	Defer(D&& deferred) :
		deferred(std::move(deferred)) {
	}

	~Defer() {
		deferred();
	}

	Defer(const Defer&)			   = delete;
	Defer& operator=(const Defer&) = delete;
};

} // namespace cero

#define defer cero::Defer CERO_CONCAT(_defer_, __LINE__) = [&]() -> void
