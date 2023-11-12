#include "Fail.hpp"

namespace cero {

namespace {

	[[noreturn]] void fail(std::source_location location) {
		std::cout << "\tFile:\t\t" << location.file_name() << '\n';
		std::cout << "\tFunction:\t" << location.function_name() << '\n';
		std::abort();
	}

} // namespace

void to_do(std::source_location location) {
	std::cout << "Not yet implemented.\n";
	fail(location);
}

void fail_unreachable(std::source_location location) {
	std::cout << "The compiler reached code that should be unreachable.\n";
	fail(location);
}

void fail_assert(std::string_view info, std::source_location location) {
	std::cout << "Assertion failed: " << info << '\n';
	fail(location);
}

void fail_result(std::string_view info, std::source_location location) {
	std::cout << "Result failed: " << info << '\n';
	fail(location);
}

} // namespace cero
