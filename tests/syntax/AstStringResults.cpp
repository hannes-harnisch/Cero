#include "syntax/ParseExhaustive.hpp"
#include "util/Test.hpp"

TEST(AstStringForSimpleFunction)
{
	auto src = make_test_source(R"_____(

main()
{}

)_____");
	auto str = parse_exhaustive(src).to_string(src);
	CHECK(str == R"_____(Printing AST for Test_AstStringForSimpleFunction
└── function `main`
    ├── parameters
    ├── return values
    └── statements
)_____");
}

TEST(AstStringForSimpleFunctionWithParametersAndReturn)
{
	auto src = make_test_source(R"_____(

a(int32 x, bool a, bool b = x) -> float32
{}

)_____");
	auto str = parse_exhaustive(src).to_string(src);
	CHECK(str == R"_____(Printing AST for Test_AstStringForSimpleFunctionWithParametersAndReturn
└── function `a`
    ├── parameters
    │   ├── in parameter `x`
    │   │   └── identifier `int32`
    │   ├── in parameter `a`
    │   │   └── identifier `bool`
    │   └── in parameter `b`
    │       ├── identifier `bool`
    │       └── identifier `x`
    ├── return values
    │   └── return value
    │       └── identifier `float32`
    └── statements
)_____");
}

TEST(AstStringForCall)
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
	CHECK(str == R"_____(Printing AST for Test_AstStringForCall
├── function `a`
│   ├── parameters
│   │   ├── in parameter `a`
│   │   │   └── identifier `int32`
│   │   ├── in parameter `f`
│   │   │   └── identifier `float64`
│   │   └── in parameter `b`
│   │       └── identifier `int64`
│   ├── return values
│   └── statements
└── function `b`
    ├── parameters
    │   ├── in parameter `i`
    │   │   └── identifier `int32`
    │   └── in parameter `f`
    │       └── identifier `float64`
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
