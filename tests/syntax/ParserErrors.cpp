#include "util/ExhaustiveReporter.hpp"
#include "util/Test.hpp"

TEST(ExpectFuncStructEnum)
{
	auto r = build_test_source(R"_____(
main()
{}

()
{}

foo()
{}
)_____");
	CHECK(r.pop_report(5, 1, Message::ExpectFuncStructEnum, "`(`"));
}

TEST(ExpectParenAfterFuncName)
{
	auto r = build_test_source(R"_____(
main)
{}
)_____");
	CHECK(r.pop_report(2, 5, Message::ExpectParenAfterFuncName, "`)`"));
}

TEST(MissingParameter)
{
	auto r = build_test_source(R"_____(
foo(, bool x) -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(2, 5, Message::ExpectType, "`,`"));
	CHECK(r.pop_report(2, 5, Message::ExpectParamName, "`,`"));
}

TEST(MissingParameterName)
{
	auto r = build_test_source(R"_____(
foo(bool, bool x) -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(2, 9, Message::ExpectParamName, "`,`"));
}

TEST(MissingParameterWithUnexpectedToken)
{
	auto r = build_test_source(R"_____(
foo(}, bool x) -> bool
{
	return x
}

goo(bool x, %) -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(2, 5, Message::ExpectType, "`}`"));
	CHECK(r.pop_report(2, 5, Message::ExpectParamName, "`}`"));
	CHECK(r.pop_report(7, 13, Message::ExpectType, "`%`"));
	CHECK(r.pop_report(7, 13, Message::ExpectParamName, "`%`"));
}

TEST(MissingParenAfterParameters)
{
	auto r = build_test_source(R"_____(
foo(bool x -> bool
{
	return x
}

goo(bool x} -> bool
{
	return x
}
)_____");
	CHECK(r.pop_report(2, 12, Message::ExpectParenAfterParams, "`->`"));
	CHECK(r.pop_report(7, 11, Message::ExpectParenAfterParams, "`}`"));
}

TEST(MissingBraceBeforeFuncBody)
{
	auto r = build_test_source(R"_____(
foo(bool x) -> bool

	return x
}

goo() -> void
}

hoo() -< void
{}
)_____");
	CHECK(r.pop_report(4, 5, Message::ExpectBraceBeforeFuncBody, "`return`"));
	CHECK(r.pop_report(8, 1, Message::ExpectBraceBeforeFuncBody, "`}`"));
	CHECK(r.pop_report(10, 7, Message::ExpectBraceBeforeFuncBody, "`-`"));
}

TEST(MissingNameInDeclaration)
{
	auto r = build_test_source(R"_____(
main()
{
    let bool x = true
	let ^bool   = &x
	let ^bool p = &x
	let ^bool   = &x
}
)_____");
	CHECK(r.pop_report(5, 17, Message::ExpectNameAfterDeclType, "`=`"));
	CHECK(r.pop_report(7, 17, Message::ExpectNameAfterDeclType, "`=`"));
}

TEST(ExpectExpr)
{
	auto r = build_test_source(R"_____(
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
	CHECK(r.pop_report(4, 5, Message::ExpectExpr, "`]`"));
	CHECK(r.pop_report(10, 1, Message::ExpectExpr, "`}`"));
	CHECK(r.pop_report(14, 5, Message::ExpectExpr, "`+=`"));
}

TEST(MissingNameAfterDot)
{
	auto r = build_test_source(R"_____(
f()
{
    x.
	var y = true

	b.()
	var z = 12
}
)_____");
	CHECK(r.pop_report(5, 5, Message::ExpectNameAfterDot, "`var`"));
	CHECK(r.pop_report(7, 7, Message::ExpectNameAfterDot, "`(`"));
}

TEST(MissingColonAfterIfCondition)
{
	auto r = build_test_source(R"_____(
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
	CHECK(r.pop_report(5, 9, Message::ExpectColonAfterCondition, "`return`"));
}

TEST(UnnecessaryColonBeforeBlock)
{
	auto r = build_test_source(R"_____(
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
	CHECK(r.pop_report(4, 9, Message::UnnecessaryColonBeforeBlock));
}

TEST(MissingParenAfterGroupExpression)
{
	auto r = build_test_source(R"_____(
f(bool a, bool b, bool c, bool d) -> bool
{
	return (a || b) && (c || d
}

g(bool a, bool b, bool c, bool d) -> bool
{
	return (a || b) && (c || d)
}
)_____");
	CHECK(r.pop_report(5, 1, Message::ExpectParenAfterGroup, "`}`"));
}

TEST(MissingParenAfterFunctionCall)
{
	auto r = build_test_source(R"_____(
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
	CHECK(r.pop_report(7, 1, Message::ExpectParenAfterCall, "`}`"));
}

TEST(MissingBracketAfterIndex)
{
	auto r = build_test_source(R"_____(
f([4]int32 x) -> int32
{
	return x[0
}

g([4]int32 x) -> int32
{
	return x[0]
}
)_____");
	CHECK(r.pop_report(5, 1, Message::ExpectBracketAfterIndex, "`}`"));
}

TEST(MissingBracketAfterArrayCount)
{
	auto r = build_test_source(R"_____(
f([4 int32 x) -> int32
{
	return x[0]
}

g([4]int32 x) -> int32
{
	return x[0]
}
)_____");
	CHECK(r.pop_report(2, 6, Message::ExpectBracketAfterArrayCount, "name `int32`"));
}
