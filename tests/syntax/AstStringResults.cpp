#include "syntax/ParseExhaustive.hpp"
#include "util/Test.hpp"

TEST(AstStringSimpleFunction)
{
	auto src = make_test_source(R"_____(
main(){}
)_____");
	auto str = parse_exhaustive(src).to_string(src);
	CHECK(str == R"_____(Printing AST for Test_AstStringSimpleFunction
└── function `main`
    ├── parameters
    ├── return values
    └── statements
)_____");
}

TEST(AstStringSimpleFunctionWithParametersAndReturn)
{
	auto src = make_test_source(R"_____(
a(int32 x, bool a, bool b = x) -> float32 {
}
)_____");
	auto str = parse_exhaustive(src).to_string(src);
	CHECK(str == R"_____(Printing AST for Test_AstStringSimpleFunctionWithParametersAndReturn
└── function `a`
    ├── parameters
    │   ├── parameter `x`
    │   │   ├── kind `In`
    │   │   └── type
    │   │       └── identifier `int32`
    │   ├── parameter `a`
    │   │   ├── kind `In`
    │   │   └── type
    │   │       └── identifier `bool`
    │   └── parameter `b`
    │       ├── kind `In`
    │       ├── type
    │       │   └── identifier `bool`
    │       └── default argument
    │           └── identifier `x`
    ├── return values
    │   └── return value
    │       └── identifier `float32`
    └── statements
)_____");
}
