#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

TEST(ExpectFuncStructEnum)
{
	ExhaustiveReporter r(R"_____(
main()
{}

()
{}

foo()
{}
)_____");
	CHECK(r.pop_report(Message::ExpectFuncStructEnum, {5, 1, test_name()}, "`(`"));
}

TEST(ExpectParenAfterFuncName)
{
	ExhaustiveReporter r(R"_____(
main)
{}
)_____");
	CHECK(r.pop_report(Message::ExpectParenAfterFuncName, {2, 5, test_name()}, "`)`"));
}

TEST(MissingParameter)
{
	ExhaustiveReporter r(R"_____(
foo(, bool x) -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(Message::ExpectType, {2, 5, test_name()}, "`,`"));
	CHECK(r.pop_report(Message::ExpectParamName, {2, 5, test_name()}, "`,`"));
}

TEST(MissingParameterName)
{
	ExhaustiveReporter r(R"_____(
foo(bool, bool x) -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(Message::ExpectParamName, {2, 9, test_name()}, "`,`"));
}

TEST(MissingParameterWithUnexpectedToken)
{
	ExhaustiveReporter r(R"_____(
foo(}, bool x) -> bool
{
	return x
}

goo(bool x, %) -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(Message::ExpectType, {2, 5, test_name()}, "`}`"));
	CHECK(r.pop_report(Message::ExpectParamName, {2, 5, test_name()}, "`}`"));
	CHECK(r.pop_report(Message::ExpectType, {7, 13, test_name()}, "`%`"));
	CHECK(r.pop_report(Message::ExpectParamName, {7, 13, test_name()}, "`%`"));
}

TEST(MissingParenAfterParameters)
{
	ExhaustiveReporter r(R"_____(
foo(bool x -> bool
{
	return x
}

goo(bool x} -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(Message::ExpectParenAfterParams, {2, 12, test_name()}, "`->`"));
	CHECK(r.pop_report(Message::ExpectParenAfterParams, {7, 11, test_name()}, "`}`"));
}

TEST(MissingBraceBeforeFuncBody)
{
	ExhaustiveReporter r(R"_____(
foo(bool x) -> bool

	return x
}

goo() -> void
}

hoo() -< void
{}
)_____");
	CHECK(r.pop_report(Message::ExpectBraceBeforeFuncBody, {4, 5, test_name()}, "`return`"));
	CHECK(r.pop_report(Message::ExpectBraceBeforeFuncBody, {8, 1, test_name()}, "`}`"));
	CHECK(r.pop_report(Message::ExpectBraceBeforeFuncBody, {10, 7, test_name()}, "`-`"));
}

TEST(MissingNameInDeclaration)
{
	ExhaustiveReporter r(R"_____(
main()
{
    let bool x = true
	let ^bool   = &x
	let ^bool p = &x
	let ^bool   = &x
}
)_____");
	CHECK(r.pop_report(Message::ExpectNameAfterDeclType, {5, 17, test_name()}, "`=`"));
	CHECK(r.pop_report(Message::ExpectNameAfterDeclType, {7, 17, test_name()}, "`=`"));
}

TEST(ExpectExpr)
{
	ExhaustiveReporter r(R"_____(
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
	CHECK(r.pop_report(Message::ExpectExpr, {4, 5, test_name()}, "`]`"));
	CHECK(r.pop_report(Message::ExpectExpr, {10, 1, test_name()}, "`}`"));
	CHECK(r.pop_report(Message::ExpectExpr, {14, 5, test_name()}, "`+=`"));
}

TEST(MissingNameAfterDot)
{
	ExhaustiveReporter r(R"_____(
f()
{
    x.
	var y = true

	b.()
	var z = 12
}
)_____");
	CHECK(r.pop_report(Message::ExpectNameAfterDot, {5, 5, test_name()}, "`var`"));
	CHECK(r.pop_report(Message::ExpectNameAfterDot, {7, 7, test_name()}, "`(`"));
}

TEST(MissingColonAfterIfCondition)
{
	ExhaustiveReporter r(R"_____(
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
	CHECK(r.pop_report(Message::ExpectColonAfterCondition, {5, 9, test_name()}, "`return`"));
}

TEST(UnnecessaryColonBeforeBlock)
{
	ExhaustiveReporter r(R"_____(
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
	CHECK(r.pop_report(Message::UnnecessaryColonBeforeBlock, {4, 9, test_name()}));
}

TEST(MissingParenAfterGroupExpression)
{
	ExhaustiveReporter r(R"_____(
f(bool a, bool b, bool c, bool d) -> bool
{
	return (a || b) && (c || d
}

g(bool a, bool b, bool c, bool d) -> bool
{
	return (a || b) && (c || d)
}
)_____");
	CHECK(r.pop_report(Message::ExpectParenAfterGroup, {5, 1, test_name()}, "`}`"));
}

TEST(MissingParenAfterFunctionCall)
{
	ExhaustiveReporter r(R"_____(
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
	CHECK(r.pop_report(Message::ExpectParenAfterCall, {7, 1, test_name()}, "`}`"));
}

TEST(MissingBracketAfterIndex)
{
	ExhaustiveReporter r(R"_____(
f([4]int32 x) -> int32
{
	return x[0
}

g([4]int32 x) -> int32
{
	return x[0]
}
)_____");
	CHECK(r.pop_report(Message::ExpectBracketAfterIndex, {5, 1, test_name()}, "`}`"));
}

TEST(MissingBracketAfterArrayCount)
{
	ExhaustiveReporter r(R"_____(
f([4 int32 x) -> int32
{
	return x[0]
}

g([4]int32 x) -> int32
{
	return x[0]
}
)_____");
	CHECK(r.pop_report(Message::ExpectBracketAfterArrayCount, {2, 6, test_name()}, "name `int32`"));
}
