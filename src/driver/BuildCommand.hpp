#pragma once

#include "driver/Config.hpp"
#include "driver/Reporter.hpp"
#include "driver/Source.hpp"

namespace cero {

bool on_build_command(const Config& config);

void build_source(const Source& source, const Config& config, Reporter& reporter);

} // namespace cero
