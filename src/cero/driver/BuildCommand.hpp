#pragma once

#include "cero/io/Configuration.hpp"
#include "cero/io/Reporter.hpp"
#include "cero/io/Source.hpp"

namespace cero {

/// Perform a build with the given configuration.
bool run_build_command(const Configuration& config);

/// Build a single source input with the given configuration and reporter.
void build_source(const Source& source, const Configuration& config, Reporter& reporter);

} // namespace cero
