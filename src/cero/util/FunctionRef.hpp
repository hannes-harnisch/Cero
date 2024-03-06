#pragma once

namespace cero {

template<typename>
class FunctionRef;

template<typename R, typename... Args>
class FunctionRef<R(Args...)> {
public:
	template<typename Fn>
	requires(!std::same_as<std::decay_t<Fn>, FunctionRef>)
	FunctionRef(Fn&& fn) noexcept :
		object_(const_cast<void*>(reinterpret_cast<const void*>(&fn))),
		fn_(fn_object_thunk<Fn>) {
	}

	FunctionRef(R (*fn)(Args...)) noexcept :
		object_(reinterpret_cast<void*>(fn)),
		fn_(fn_ptr_thunk) {
	}

	template<typename M, typename C>
	FunctionRef(M C::*) = delete;

	[[nodiscard]] R operator()(Args... args) const {
		return fn_(object_, std::forward<Args>(args)...);
	}

private:
	void* object_;
	R (*fn_)(void*, Args...);

	template<typename Fn>
	static R fn_object_thunk(void* object, Args... args) {
		return (*reinterpret_cast<Fn*>(object))(std::forward<Args>(args)...);
	}

	static R fn_ptr_thunk(void* object, Args... args) {
		return reinterpret_cast<R (*)(Args...)>(object)(std::forward<Args>(args)...);
	}
};

} // namespace cero
