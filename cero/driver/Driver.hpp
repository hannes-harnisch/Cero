#pragma once

#include "driver/ExitCode.hpp"

#include <span>
#include <string_view>

ExitCode run_driver(std::span<std::string_view> args);
