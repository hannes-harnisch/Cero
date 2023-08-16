#pragma once

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

	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;
};

} // namespace cero