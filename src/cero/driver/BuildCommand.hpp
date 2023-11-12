#pragma once

#include "cero/io/Config.hpp"
#include "cero/io/Reporter.hpp"
#include "cero/io/Source.hpp"

namespace cero {

bool run_build_command(const Config& config);

void build_source(const Source& source, const Config& config, Reporter& reporter);

} // namespace cero
