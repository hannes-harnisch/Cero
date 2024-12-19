#include "SystemError.hpp"

#include "cero/util/WinApi.win.hpp"

std::error_condition cero::get_last_system_error() {
	const DWORD last_error = ::GetLastError();
	return std::system_category().default_error_condition(last_error);
}
