#pragma once
#include <source_location>
// To be used in control flow branches that are only reachable via logic bugs in the compiler. Terminates the compiler in any
// build configuration.
[[noreturn]] void fail_unreachable();

// To be used in place of functionality that is not yet implemented. Terminates the compiler in any build configuration.
[[noreturn]] void to_do(std::source_location location = std::source_location::current());
