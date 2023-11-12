#pragma once

namespace cero {

template<typename>
class FuncRef;

template<typename R, typename... Args>
class FuncRef<R(Args...)> {
public:
	template<typename Fn>
	FuncRef(Fn&& fn) noexcept :
		object_(const_cast<void*>(static_cast<const void*>(&fn))),
		fn_([](void* object, Args... args) -> R {
		return std::invoke(*static_cast<Fn*>(object), std::forward<Args>(args)...);
	}) {
	}

	R operator()(Args... args) const {
		return fn_(object_, std::forward<Args>(args)...);
	}

private:
	void* object_;
	R (*fn_)(void*, Args...);
};

} // namespace cero
