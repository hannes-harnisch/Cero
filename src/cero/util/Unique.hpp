#pragma once

#include <utility>

namespace cero {

template<typename T, void Destroy(T), T Null()>
class Unique {
public:
	Unique() :
		resource_(Null()) {
	}

	explicit Unique(T resource) :
		resource_(resource) {
	}

	Unique(Unique&& other) noexcept :
		resource_(std::exchange(other.resource_, Null())) {
	}

	~Unique() {
		if (resource_ != Null()) {
			Destroy(resource_);
		}
	}

	Unique& operator=(Unique&& other) noexcept {
		reset(std::exchange(other.resource_, Null()));
		return *this;
	}

	T get() const noexcept {
		return resource_;
	}

	void reset(T resource = Null()) noexcept {
		if (resource_ != Null()) {
			Destroy(resource_);
		}
		resource_ = resource;
	}

	T* operator->() noexcept {
		return &resource_;
	}

	const T* operator->() const noexcept {
		return &resource_;
	}

private:
	T resource_;
};

} // namespace cero
