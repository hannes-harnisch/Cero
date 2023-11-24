#pragma once

namespace cero {

template<typename D>
class Defer {
public:
	Defer(D&& deferred) :
		deferred_(std::move(deferred)) {
	}

	~Defer() {
		deferred_();
	}

	Defer(const Defer&) = delete;
	Defer& operator=(const Defer&) = delete;

private:
	D deferred_;
};

} // namespace cero
