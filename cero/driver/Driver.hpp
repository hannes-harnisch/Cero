#pragma once

#include "driver/ExitCode.hpp"

#include <span>
#include <string_view>

namespace cero
{

ExitCode run_driver(std::span<std::string_view> args);

}