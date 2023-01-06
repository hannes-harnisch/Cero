#pragma once

#include "driver/Config.hpp"
#include "driver/Reporter.hpp"
#include "driver/Source.hpp"

namespace cero
{

Reporter build_source(const Source& source, const Config& config);

}
