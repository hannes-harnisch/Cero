#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

#include <cero/syntax/Parse.hpp>

TEST(AstStringForSimpleFunction)
{
	auto source = make_test_source(R"_____(

main()
{}

)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);
	CHECK(ast.to_string(source) == R"_____(Printing AST for Test_AstStringForSimpleFunction
└── function `main`
    ├── parameters
    ├── outputs
    └── statements
)_____");
}

TEST(AstStringForSimpleFunctionWithParametersAndReturn)
{
	auto source = make_test_source(R"_____(

a(int32 x, bool _a, bool _b = x) -> float32
{}

)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);
	CHECK(ast.to_string(source) == R"_____(Printing AST for Test_AstStringForSimpleFunctionWithParametersAndReturn
└── function `a`
    ├── parameters
    │   ├── in parameter `x`
    │   │   └── identifier `int32`
    │   ├── in parameter `_a`
    │   │   └── identifier `bool`
    │   └── in parameter `_b`
    │       ├── identifier `bool`
    │       └── identifier `x`
    ├── outputs
    │   └── output
    │       └── identifier `float32`
    └── statements
)_____");
}

TEST(AstStringForCall)
{
	auto source = make_test_source(R"_____(

a(int32 _a, float64 _f, int64 _b)
{}

b(int32 i, float64 f)
{
	a(i, f, i * i)
}

)_____");

	ExhaustiveReporter r;

	auto ast = cero::parse(source, r);
	CHECK(ast.to_string(source) == R"_____(Printing AST for Test_AstStringForCall
├── function `a`
│   ├── parameters
│   │   ├── in parameter `_a`
│   │   │   └── identifier `int32`
│   │   ├── in parameter `_f`
│   │   │   └── identifier `float64`
│   │   └── in parameter `_b`
│   │       └── identifier `int64`
│   ├── outputs
│   └── statements
└── function `b`
    ├── parameters
    │   ├── in parameter `i`
    │   │   └── identifier `int32`
    │   └── in parameter `f`
    │       └── identifier `float64`
    ├── outputs
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
