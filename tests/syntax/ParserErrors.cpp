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
