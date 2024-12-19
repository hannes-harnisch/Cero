#include "SystemError.hpp"

std::error_condition cero::get_last_system_error() {
	const int last_error = errno;
	return std::system_category().default_error_condition(last_error);
}
