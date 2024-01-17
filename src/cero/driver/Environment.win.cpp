#include "Environment.hpp"

#include "cero/util/WinApi.win.hpp"

namespace cero {

void initialize_environment() {
	::SetConsoleCP(CP_UTF8);
	::SetConsoleOutputCP(CP_UTF8);
}

} // namespace cero
