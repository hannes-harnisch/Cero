#pragma once

namespace cero {

template<typename T, size_t Size, size_t Align = alignof(void*)>
class UniqueImpl {
public:
	UniqueImpl() { // NOLINT(*-pro-type-member-init)
		static_assert_size_align();
		new (storage_) T();
	}

	explicit(false) UniqueImpl(T&& t) { // NOLINT(*-pro-type-member-init)
		static_assert_size_align();
		new (storage_) T(std::move(t));
	}

	UniqueImpl(UniqueImpl&& other) noexcept { // NOLINT(*-pro-type-member-init)
		static_assert_size_align();
		new (storage_) T(std::exchange(*other, T()));
	}

	~UniqueImpl() {
		(*this)->destroy();
	}

	UniqueImpl& operator=(UniqueImpl&& other) noexcept {
		if (this != &other) {
			(*this)->destroy();
			**this = std::exchange(*other, T());
		}
		return *this;
	}

	T& operator*() & noexcept {
		return *std::launder(reinterpret_cast<T*>(storage_));
	}

	const T& operator*() const& noexcept {
		return *std::launder(reinterpret_cast<const T*>(storage_));
	}

	T&& operator*() && noexcept {
		return std::move(*std::launder(reinterpret_cast<T*>(storage_)));
	}

	T* operator->() noexcept {
		return std::launder(reinterpret_cast<T*>(storage_));
	}

	const T* operator->() const noexcept {
		return std::launder(reinterpret_cast<const T*>(storage_));
	}

private:
	alignas(Align) unsigned char storage_[Size];

	static void static_assert_size_align() {
		static_assert(Size >= sizeof(T) && Align >= alignof(T), "Specified size and alignment must be greater or equal to the "
																"size and alignment of T.");
	}
};

} // namespace cero
