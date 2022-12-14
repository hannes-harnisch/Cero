#include "util/windows/Windows.API.hpp"

void init_platform()
{
	::SetConsoleOutputCP(CP_UTF8);
}
