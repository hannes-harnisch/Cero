#pragma once

#include <string>
#include <string_view>

namespace cero::windows {

std::wstring widen_string(std::string_view str);

}