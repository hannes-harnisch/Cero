#include "syntax/ParseExhaustive.hpp"
#include "util/Test.hpp"

TEST(AstStringSimpleFunction)
{
	auto src = make_test_source(R"_____(

main()
{}

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

a(int32 x, bool a, bool b = x) -> float32
{}

)_____");
	auto str = parse_exhaustive(src).to_string(src);
	CHECK(str == R"_____(Printing AST for Test_AstStringSimpleFunctionWithParametersAndReturn
└── function `a`
    ├── parameters
    │   ├── parameter `x`
    │   │   ├── kind `in`
    │   │   └── type
    │   │       └── identifier `int32`
    │   ├── parameter `a`
    │   │   ├── kind `in`
    │   │   └── type
    │   │       └── identifier `bool`
    │   └── parameter `b`
    │       ├── kind `in`
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

TEST(AstStringCall)
{
	auto src = make_test_source(R"_____(

a(int32 a, float64 f, int64 b)
{}

b(int32 i, float64 f)
{
	a(i, f, i * i)
}

)_____");
	auto str = parse_exhaustive(src).to_string(src);
	CHECK(str == R"_____(Printing AST for Test_AstStringCall
├── function `a`
│   ├── parameters
│   │   ├── parameter `a`
│   │   │   ├── kind `in`
│   │   │   └── type
│   │   │       └── identifier `int32`
│   │   ├── parameter `f`
│   │   │   ├── kind `in`
│   │   │   └── type
│   │   │       └── identifier `float64`
│   │   └── parameter `b`
│   │       ├── kind `in`
│   │       └── type
│   │           └── identifier `int64`
│   ├── return values
│   └── statements
└── function `b`
    ├── parameters
    │   ├── parameter `i`
    │   │   ├── kind `in`
    │   │   └── type
    │   │       └── identifier `int32`
    │   └── parameter `f`
    │       ├── kind `in`
    │       └── type
    │           └── identifier `float64`
    ├── return values
    └── statements
        └── call expression
            ├── identifier `a`
            └── arguments
                ├── identifier `i`
                ├── identifier `f`
                └── `*`
                    ├── identifier `i`
                    └── identifier `i`
)_____");
}
