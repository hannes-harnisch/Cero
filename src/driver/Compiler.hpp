#pragma once

#include <span>
#include <string_view>

namespace cero {

bool run_compiler(std::span<std::string_view> args);

}
