#include "Fail.hpp"

#include "cero/util/Macros.hpp"

namespace cero {

[[noreturn]] static void fail(std::source_location location) {
	fmt::println("\tFile:     {}", location.file_name());
	fmt::println("\tFunction: {}", location.function_name());
	std::abort();
}

} // namespace cero

void cero::to_do(std::source_location location) {
	CERO_DEBUG_BREAK();
	fmt::println("Not yet implemented.");
	fail(location);
}

void cero::fail_unreachable(std::source_location location) {
	CERO_DEBUG_BREAK();
	fmt::println("The compiler reached code that should be unreachable.");
	fail(location);
}

void cero::fail_check(std::string_view msg, std::source_location location) {
	CERO_DEBUG_BREAK();
	fmt::println("Requirement failed: {}", msg);
	fail(location);
}

void cero::check(bool condition, std::string_view msg, std::source_location location) {
	if (!condition) {
		CERO_DEBUG_BREAK();
		fmt::println("Requirement failed: {}", msg);
		fail(location);
	}
}
