#include "Fail.hpp"

namespace {

[[noreturn]] void fail(std::source_location location) {
	std::cout << "\tFile:\t\t" << location.file_name() << std::endl;
	std::cout << "\tFunction:\t" << location.function_name() << std::endl;
	std::abort();
}

} // namespace

namespace cero {

void to_do(std::source_location location) {
	std::cout << "Not yet implemented." << std::endl;
	fail(location);
}

void fail_unreachable(std::source_location location) {
	std::cout << "The compiler reached code that should be unreachable." << std::endl;
	fail(location);
}

void fail_assert(std::string_view info, std::source_location location) {
	std::cout << "Assertion failed: " << info << std::endl;
	fail(location);
}

void fail_result(std::string_view info, std::source_location location) {
	std::cout << "Result failed: " << info << std::endl;
	fail(location);
}

} // namespace cero
