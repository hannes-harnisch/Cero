#pragma once

#include "cero/util/Fail.hpp"

namespace cero {

// Simple implementation of a monadic result type, holding either a success value T or an error E.
template<typename T, typename E>
class Result {
public:
	Result(const T& value) :
		has_value_(true),
		value_(value) {
	}

	Result(T&& value) noexcept :
		has_value_(true),
		value_(std::move(value)) {
	}

	Result(const E& error) :
		has_value_(false),
		error_(error) {
	}

	Result(E&& error) noexcept :
		has_value_(false),
		error_(std::move(error)) {
	}

	Result(const Result& other) :
		has_value_(other.has_value_) {
		if (has_value_) {
			new (&value_) T(other.value_);
		} else {
			new (&error_) E(other.error_);
		}
	}

	Result(Result&& other) noexcept :
		has_value_(other.has_value_) {
		if (has_value_) {
			new (&value_) T(std::move(other.value_));
		} else {
			new (&error_) E(std::move(other.error_));
		}
	}

	~Result() {
		if (has_value_) {
			value_.~T();
		} else {
			error_.~E();
		}
	}

	Result& operator=(const Result& other) {
		if (has_value_) {
			if (other.has_value_) {
				value_ = other.value_;
			} else {
				value_.~T();
				new (&error_) E(other.error_);
			}
		} else {
			if (other.has_value_) {
				error_.~E();
				new (&value_) T(other.value_);
			} else {
				error_ = other.error_;
			}
		}
		has_value_ = other.has_value_;
		return *this;
	}

	Result& operator=(Result&& other) noexcept {
		if (has_value_) {
			if (other.has_value_) {
				value_ = std::move(other.value_);
			} else {
				value_.~T();
				new (&error_) E(std::move(other.error_));
			}
		} else {
			if (other.has_value_) {
				error_.~E();
				new (&value_) T(std::move(other.value_));
			} else {
				error_ = std::move(other.error_);
			}
		}
		has_value_ = other.has_value_;
		return *this;
	}

	bool has_value() const noexcept {
		return has_value_;
	}

	explicit operator bool() const noexcept {
		return has_value_;
	}

	T& value() & noexcept {
		if (has_value_) {
			return value_;
		}
		on_value_access_fail();
	}

	const T& value() const& noexcept {
		if (has_value_) {
			return value_;
		}
		on_value_access_fail();
	}

	T&& value() && noexcept {
		if (has_value_) {
			return std::move(value_);
		}
		on_value_access_fail();
	}

	E& error() & noexcept {
		if (has_value_) {
			on_error_access_fail();
		}
		return error_;
	}

	const E& error() const& noexcept {
		if (has_value_) {
			on_error_access_fail();
		}
		return error_;
	}

	E&& error() && noexcept {
		if (has_value_) {
			on_error_access_fail();
		}
		return std::move(error_);
	}

	T* get() noexcept {
		if (has_value_) {
			return &value_;
		}
		return nullptr;
	}

	const T* get() const noexcept {
		if (has_value_) {
			return &value_;
		}
		return nullptr;
	}

	E* get_error() noexcept {
		if (has_value_) {
			return nullptr;
		}
		return &error_;
	}

	const E* get_error() const noexcept {
		if (has_value_) {
			return nullptr;
		}
		return &error_;
	}

	template<typename Fn>
	Result<std::invoke_result_t<Fn, T>, E> transform(Fn&& fn) & {
		if (has_value_) {
			return std::forward<Fn>(fn)(value_);
		} else {
			return error_;
		}
	}

	template<typename Fn>
	Result<std::invoke_result_t<Fn, T>, E> transform(Fn&& fn) const& {
		if (has_value_) {
			return std::forward<Fn>(fn)(value_);
		} else {
			return error_;
		}
	}

	template<typename Fn>
	Result<std::invoke_result_t<Fn, T>, E> transform(Fn&& fn) && {
		if (has_value_) {
			return std::forward<Fn>(fn)(std::move(value_));
		} else {
			return std::move(error_);
		}
	}

private:
	bool has_value_;

	union {
		T value_;
		E error_;
	};

	[[noreturn]] static void on_value_access_fail() {
		fail_assert("tried to access success value of an error result");
	}

	[[noreturn]] static void on_error_access_fail() {
		fail_assert("tried to access error value of a successful result");
	}
};

} // namespace cero
