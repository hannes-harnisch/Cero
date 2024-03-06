#pragma once

#include <source_location>
#include <string_view>

namespace cero {

/// Terminates the compiler immediately and unconditionally. To be used in place of functionality that is not yet implemented.
[[noreturn]] void to_do(std::source_location location = std::source_location::current());

/// Terminates the compiler immediately and unconditionally. To be used in control flow branches that are only reachable if
/// there are logic bugs in the compiler.
[[noreturn]] void fail_unreachable(std::source_location location = std::source_location::current());

/// Terminates the compiler immediately and unconditionally. To be used when an expected precondition or invariant was not
/// upheld.
[[noreturn]] void fail_assert(std::string_view info, std::source_location location = std::source_location::current());

/// Terminates the compiler immediately and unconditionally. To be used when an expected postcondition or outcome was not
/// reached.
[[noreturn]] void fail_result(std::string_view info, std::source_location location = std::source_location::current());

} // namespace cero
