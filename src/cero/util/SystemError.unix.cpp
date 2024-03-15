#include "SystemError.hpp"

namespace cero {

std::error_code get_system_error() {
	return std::error_code(errno, std::system_category());
}

std::string get_system_error_message(std::error_code error_code) {
	return error_code.message();
}

} // namespace cero
