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
)_____");
	CHECK(r.pop_report(Message::ExpectBraceBeforeFuncBody, {4, 5, test_name()}, "`return`"));
	CHECK(r.pop_report(Message::ExpectBraceBeforeFuncBody, {8, 1, test_name()}, "`}`"));
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

b()
{
    += x
}
)_____");
	CHECK(r.pop_report(Message::ExpectExpr, {4, 5, test_name()}, "`]`"));
	CHECK(r.pop_report(Message::ExpectExpr, {9, 5, test_name()}, "`+=`"));
}

TEST(MissingNameAfterDot)
{
	ExhaustiveReporter r(R"_____(
a()
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
