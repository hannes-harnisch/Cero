#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

CERO_TEST(ExpectFuncStructEnum)
{
	ExhaustiveReporter r;
	r.expect(5, 1, cero::Message::ExpectFuncStructEnum, "`(`");

	build_test_source(r, R"_____(
main()
{}

()
{}

foo()
{}
)_____");
}

CERO_TEST(ExpectParenAfterFuncName)
{
	ExhaustiveReporter r;
	r.expect(2, 5, cero::Message::ExpectParenAfterFuncName, "`)`");

	build_test_source(r, R"_____(
main)
{}
)_____");
}

CERO_TEST(MissingParameter)
{
	ExhaustiveReporter r;
	r.expect(2, 5, cero::Message::ExpectType, "`,`");
	r.expect(2, 5, cero::Message::ExpectParamName, "`,`");

	build_test_source(r, R"_____(
foo(, bool x) -> bool
{
	return x
}
)_____");
}

CERO_TEST(MissingParameterName)
{
	ExhaustiveReporter r;
	r.expect(2, 9, cero::Message::ExpectParamName, "`,`");

	build_test_source(r, R"_____(
foo(bool, bool x) -> bool
{
	return x
}
)_____");
}

CERO_TEST(MissingParameterWithUnexpectedToken)
{
	ExhaustiveReporter r;
	r.expect(2, 5, cero::Message::ExpectType, "`}`");
	r.expect(2, 5, cero::Message::ExpectParamName, "`}`");
	r.expect(7, 13, cero::Message::ExpectType, "`%`");
	r.expect(7, 13, cero::Message::ExpectParamName, "`%`");

	build_test_source(r, R"_____(
foo(}, bool x) -> bool
{
	return x
}

goo(bool x, %) -> bool
{
	return x
}
)_____");
}

CERO_TEST(MissingParenAfterParameters)
{
	ExhaustiveReporter r;
	r.expect(2, 12, cero::Message::ExpectParenAfterParams, "`->`");
	r.expect(7, 11, cero::Message::ExpectParenAfterParams, "`}`");

	build_test_source(r, R"_____(
foo(bool x -> bool
{
	return x
}

goo(bool x} -> bool
{
	return x
}
)_____");
}

CERO_TEST(MissingBraceBeforeFuncBody)
{
	ExhaustiveReporter r;
	r.expect(4, 5, cero::Message::ExpectBraceBeforeFuncBody, "`return`");
	r.expect(8, 1, cero::Message::ExpectBraceBeforeFuncBody, "`}`");
	r.expect(10, 7, cero::Message::ExpectBraceBeforeFuncBody, "`-`");

	build_test_source(r, R"_____(
foo(bool x) -> bool

	return x
}

goo() -> void
}

hoo() -< void
{}
)_____");
}

CERO_TEST(MissingNameInDeclaration)
{
	ExhaustiveReporter r;
	r.expect(5, 17, cero::Message::ExpectNameAfterDeclType, "`=`");
	r.expect(7, 17, cero::Message::ExpectNameAfterDeclType, "`=`");

	build_test_source(r, R"_____(
main()
{
    let bool x = true
	let ^bool   = &x
	let ^bool p = &x
	let ^bool   = &x
}
)_____");
}

CERO_TEST(ExpectExpr)
{
	ExhaustiveReporter r;
	r.expect(4, 5, cero::Message::ExpectExpr, "`]`");
	r.expect(10, 1, cero::Message::ExpectExpr, "`}`");
	r.expect(14, 5, cero::Message::ExpectExpr, "`+=`");

	build_test_source(r, R"_____(
a()
{
    ]
}

c()
{
	foo(
}

b()
{
    += x
}

foo() {}
)_____");
}

CERO_TEST(MissingNameAfterDot)
{
	ExhaustiveReporter r;
	r.expect(5, 5, cero::Message::ExpectNameAfterDot, "`var`");
	r.expect(7, 7, cero::Message::ExpectNameAfterDot, "`(`");

	build_test_source(r, R"_____(
f()
{
    x.
	var y = true

	b.()
	var z = 12
}
)_____");
}

CERO_TEST(MissingColonAfterIfCondition)
{
	ExhaustiveReporter r;
	r.expect(5, 9, cero::Message::ExpectColonOrBlock, "`return`");

	build_test_source(r, R"_____(
f(bool b) -> int32
{
	if b
		return 4

	print(b)
	print(b)
	print(b)
}

g(bool b) -> int32
{
	if b:
		return 4
	else
		return 5
}
)_____");
}

CERO_TEST(UnnecessaryColonBeforeBlock)
{
	ExhaustiveReporter r;
	r.expect(4, 9, cero::Message::UnnecessaryColonBeforeBlock);

	build_test_source(r, R"_____(
f(bool b) -> int32
{
	if b:
	{
		return 4
	}
}

g(bool b) -> int32
{
	if b
	{
		return 4
	}
}
)_____");
}

CERO_TEST(MissingParenAfterGroupExpression)
{
	ExhaustiveReporter r;
	r.expect(5, 1, cero::Message::ExpectClosingParen, "`}`");

	build_test_source(r, R"_____(
f(bool a, bool b, bool c, bool d) -> bool
{
	return (a || b) && (c || d
}

g(bool a, bool b, bool c, bool d) -> bool
{
	return (a || b) && (c || d)
}
)_____");
}

CERO_TEST(MissingParenAfterFunctionCall)
{
	ExhaustiveReporter r;
	r.expect(7, 1, cero::Message::ExpectClosingParen, "`}`");

	build_test_source(r, R"_____(
foo(int32 _) {}

f()
{
	foo(1
}

g()
{
	foo(2)
}
)_____");
}

CERO_TEST(MissingBracketAfterIndex)
{
	ExhaustiveReporter r;
	r.expect(5, 1, cero::Message::ExpectBracketAfterIndex, "`}`");

	build_test_source(r, R"_____(
f([4]int32 x) -> int32
{
	return x[0
}

g([4]int32 x) -> int32
{
	return x[0]
}
)_____");
}

CERO_TEST(MissingBracketAfterArrayBound)
{
	ExhaustiveReporter r;
	r.expect(2, 6, cero::Message::ExpectBracketAfterArrayBound, "name `int32`");

	build_test_source(r, R"_____(
f([4 int32 x) -> int32
{
	return x[0]
}

g([4]int32 x) -> int32
{
	return x[0]
}
)_____");
}

CERO_TEST(ExpectBraceAfterVariability)
{
	ExhaustiveReporter r;
	r.expect(2, 10, cero::Message::ExpectBraceAfterVariability, "name `MyList`");

	build_test_source(r, R"_____(
f(^var{1 MyList l) -> int32
{
	let int32 p = l^[0]
	return p
}

g(^var{1} MyList p) -> int32
{
	let int32 p = l^[0]
	return p
}
)_____");
}

CERO_TEST(ExpectArrowAfterFuncTypeParams)
{
	ExhaustiveReporter r;
	r.expect(2, 12, cero::Message::ExpectArrowAfterFuncTypeParams, "name `f`");

	build_test_source(r, R"_____(
a(^(int32) f) -> int32
{
	return f()
}
)_____");
}

CERO_TEST(FuncTypeDefaultArgument) // TODO: add check for this in expression context
{
	ExhaustiveReporter r;
	r.expect(2, 13, cero::Message::FuncTypeDefaultArgument);

	build_test_source(r, R"_____(
a(^(int32 x = 0)->int32 f) -> int32
{
	return f(3)
}
)_____");
}

CERO_TEST(AmbiguousOperatorChaining)
{
	ExhaustiveReporter r;
	r.expect(4, 20, cero::Message::AmbiguousOperatorChaining, "==");
	r.expect(4, 25, cero::Message::AmbiguousOperatorChaining, "==");

	build_test_source(r, R"_____(
f(bool a, bool b, bool c, bool d)
{
	let e = a == b == c == d
}
)_____");
}

CERO_TEST(AmbiguousOperatorMixing)
{
	ExhaustiveReporter r;
	r.expect(4, 19, cero::Message::AmbiguousOperatorMixing, ">", "==");
	r.expect(4, 24, cero::Message::AmbiguousOperatorMixing, "==", "<");

	build_test_source(r, R"_____(
f(bool a, bool b, bool c, bool d)
{
	let e = a > b == c < d
}
)_____");
}
