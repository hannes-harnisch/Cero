#pragma once

#include "cero/driver/Config.hpp"
#include "cero/driver/Reporter.hpp"
#include "cero/driver/Source.hpp"

namespace cero
{

void build_source(const Source& source, const Config& config, Reporter& reporter);

}
