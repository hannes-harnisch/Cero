#include "common/Test.hpp"

#include <cero/util/FunctionRef.hpp>

namespace tests {

CERO_TEST(FunctionRefCapturingLambda) {
	int32_t i = 0;
	cero::FunctionRef<void()>([&] {
		++i;
	})();
	CHECK_EQ(i, 1);
}

CERO_TEST(FunctionRefParameterLambda) {
	cero::FunctionRef<int32_t(int32_t, int32_t)> f = [](int32_t i, int32_t j) -> int32_t {
		return i + j;
	};

	int32_t i = 1;
	i = f(i, i);
	CHECK_EQ(i, 2);
}

void func(int32_t& i) {
	++i;
}

CERO_TEST(FunctionRefFunction) {
	cero::FunctionRef<void(int32_t&)> f = func;

	int32_t i = 0;
	f(i);
	CHECK_EQ(i, 1);
}

} // namespace tests
