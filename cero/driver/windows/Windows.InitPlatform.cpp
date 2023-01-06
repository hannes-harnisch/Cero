#include "util/windows/Windows.API.hpp"

namespace cero
{

void init_platform()
{
	::SetConsoleOutputCP(CP_UTF8);
}

} // namespace cero