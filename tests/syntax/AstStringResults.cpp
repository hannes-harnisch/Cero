#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/syntax/Parse.hpp>

CERO_TEST(AstStringForSimpleFunction) {
	auto source = make_test_source(R"_____(

main()
{}

)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	auto str = ast.to_string(source);
	auto expected = R"_____(Printing AST for AstStringForSimpleFunction
└── function `main`
    ├── parameters
    ├── outputs
    └── statements
)_____";
	CHECK(str == expected);
}

CERO_TEST(AstStringForSimpleFunctionWithParametersAndReturn) {
	auto source = make_test_source(R"_____(

a(int32 x, bool _a, bool _b = x) -> float32
{}

)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	auto str = ast.to_string(source);
	auto expected = R"_____(Printing AST for AstStringForSimpleFunctionWithParametersAndReturn
└── function `a`
    ├── parameters
    │   ├── value parameter `x`
    │   │   └── name `int32`
    │   ├── value parameter `_a`
    │   │   └── name `bool`
    │   └── value parameter `_b`
    │       ├── name `bool`
    │       └── name `x`
    ├── outputs
    │   └── output
    │       └── name `float32`
    └── statements
)_____";
	CHECK(str == expected);
}

CERO_TEST(AstStringForCall) {
	auto source = make_test_source(R"_____(

a(int32 _a, float64 _f, int64 _b)
{}

b(int32 i, float64 f)
{
	a(i, f, i * i);
}

)_____");

	ExhaustiveReporter r;
	auto ast = cero::parse(source, r);

	auto str = ast.to_string(source);
	auto expected = R"_____(Printing AST for AstStringForCall
├── function `a`
│   ├── parameters
│   │   ├── value parameter `_a`
│   │   │   └── name `int32`
│   │   ├── value parameter `_f`
│   │   │   └── name `float64`
│   │   └── value parameter `_b`
│   │       └── name `int64`
│   ├── outputs
│   └── statements
└── function `b`
    ├── parameters
    │   ├── value parameter `i`
    │   │   └── name `int32`
    │   └── value parameter `f`
    │       └── name `float64`
    ├── outputs
    └── statements
        └── call expression
            ├── name `a`
            └── arguments
                ├── name `i`
                ├── name `f`
                └── `*`
                    ├── name `i`
                    └── name `i`
)_____";
	CHECK(str == expected);
}
