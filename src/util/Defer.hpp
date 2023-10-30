#pragma once

namespace cero {

template<typename D>
class Defer {
public:
	Defer(D&& deferred) :
		deferred(std::move(deferred)) {
	}

	~Defer() {
		deferred();
	}

	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;

private:
	D deferred;
};

} // namespace cero