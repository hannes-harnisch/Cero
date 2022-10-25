#pragma once

#include "driver/ExitCode.hpp"
#include "driver/Reporter.hpp"
#include "driver/Source.hpp"

#include <span>
#include <string_view>

ExitCode run_driver(std::span<std::string_view> args);

Reporter build_file(const Source& source);
