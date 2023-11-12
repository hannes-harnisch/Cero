#pragma once

#include <string>
#include <string_view>

namespace cero::windows {

std::wstring utf8_to_utf16(std::string_view input);
std::string utf16_to_utf8(std::wstring_view input);

} // namespace cero::windows
