#include "util/windows/Windows.API.hpp"

namespace cero::windows
{

void initialize_system_state()
{
	::SetConsoleOutputCP(CP_UTF8);
}

} // namespace cero::windows