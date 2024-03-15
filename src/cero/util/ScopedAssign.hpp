#pragma once

namespace cero {

/// Takes an object and assigns it a new value after remembering its old value. The object's old value is restored at the end of
/// the scope.
template<typename T>
class ScopedAssign {
public:
	template<typename U>
	ScopedAssign(T& object, U&& new_value) :
		object_(object),
		old_value_(std::exchange(object, std::forward<U>(new_value))) {
	}

	~ScopedAssign() {
		object_ = std::move(old_value_);
	}

	ScopedAssign(ScopedAssign&&) = delete;
	ScopedAssign& operator=(ScopedAssign&&) = delete;

private:
	T& object_;
	T old_value_;
};

} // namespace cero
