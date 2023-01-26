#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

TEST(AstStringForSimpleFunction)
{
	ExhaustiveReporter r;

	auto src = make_test_source(R"_____(

main()
{}

)_____");

	auto ast = cero::parse(src, r);
	CHECK(ast.to_string(src) == R"_____(Printing AST for Test_AstStringForSimpleFunction
└── function `main`
    ├── parameters
    ├── return values
    └── statements
)_____");
}

TEST(AstStringForSimpleFunctionWithParametersAndReturn)
{
	ExhaustiveReporter r;

	auto src = make_test_source(R"_____(

a(int32 x, bool a, bool b = x) -> float32
{}

)_____");

	auto ast = cero::parse(src, r);
	CHECK(ast.to_string(src) == R"_____(Printing AST for Test_AstStringForSimpleFunctionWithParametersAndReturn
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
	ExhaustiveReporter r;

	auto src = make_test_source(R"_____(

a(int32 a, float64 f, int64 b)
{}

b(int32 i, float64 f)
{
	a(i, f, i * i)
}

)_____");

	auto ast = cero::parse(src, r);
	CHECK(ast.to_string(src) == R"_____(Printing AST for Test_AstStringForCall
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
