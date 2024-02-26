#include "Fail.hpp"

namespace cero {

namespace {

	[[noreturn]] void fail(std::source_location location) {
		fmt::println("\tFile:     {}", location.file_name());
		fmt::println("\tFunction: {}", location.function_name());
		std::abort();
	}

} // namespace

void to_do(std::source_location location) {
	fmt::println("Not yet implemented.");
	fail(location);
}

void fail_unreachable(std::source_location location) {
	fmt::println("The compiler reached code that should be unreachable.");
	fail(location);
}

void fail_assert(std::string_view info, std::source_location location) {
	fmt::println("Assertion failed: {}", info);
	fail(location);
}

void fail_result(std::string_view info, std::source_location location) {
	fmt::println("Result failed: {}", info);
	fail(location);
}

} // namespace cero
