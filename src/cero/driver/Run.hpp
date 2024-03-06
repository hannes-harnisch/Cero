#pragma once

#include <span>

namespace cero {

/// Entry point for the compiler with the given command line arguments.
bool run(std::span<char*> args);

} // namespace cero
