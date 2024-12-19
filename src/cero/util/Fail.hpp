#pragma once

#include "Macros.hpp"

#include <source_location>
#include <string_view>

namespace cero {

/// Terminates the compiler immediately and unconditionally. To be used in place of functionality that is not yet implemented.
[[noreturn]] void to_do(std::source_location location = std::source_location::current());

/// Terminates the compiler immediately and unconditionally. To be used in control flow branches that are only reachable if
/// there are logic bugs in the compiler.
[[noreturn]] void fail_unreachable(std::source_location location = std::source_location::current());

/// Terminates the compiler immediately and unconditionally. To be used when an expected precondition, result or invariant
/// was not upheld.
[[noreturn]] void fail_check(std::string_view msg, std::source_location location = std::source_location::current());

/// Terminates the compiler immediately if the condition is false. To be used when an expected precondition, result or invariant
/// was not upheld.
void check(bool condition, std::string_view msg, std::source_location location = std::source_location::current());

} // namespace cero
