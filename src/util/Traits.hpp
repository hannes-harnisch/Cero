#pragma once

#include <source_location>
#include <string_view>

namespace cero {

std::string_view normalize_function_name(std::source_location location);

template<typename...>
constexpr inline bool always_false = false;

} // namespace cero
