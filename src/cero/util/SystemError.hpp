#pragma once

namespace cero {

std::error_code get_system_error();

std::string get_system_error_message(std::error_code error_code = get_system_error());

} // namespace cero
