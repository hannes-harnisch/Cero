#pragma once

#include <source_location>
#include <string_view>

namespace cero {

// To be used in place of functionality that is not yet implemented. Terminates the compiler in any build configuration.
[[noreturn]] void to_do(std::source_location location = std::source_location::current());

// To be used in control flow branches that are only reachable via logic bugs in the compiler. Terminates the compiler in any
// build configuration.
[[noreturn]] void fail_unreachable(std::source_location location = std::source_location::current());

// To be used when an expected precondition or invariant was not upheld. Terminates the compiler in any build configuration.
[[noreturn]] void fail_assert(std::string_view info, std::source_location location = std::source_location::current());

// To be used when an expected postcondition or outcome was not reached. Terminates the compiler in any build configuration.
[[noreturn]] void fail_result(std::string_view info, std::source_location location = std::source_location::current());

} // namespace cero
