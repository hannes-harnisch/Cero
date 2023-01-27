#include "cero/driver/Initialize.hpp"

namespace cero
{

namespace
#ifdef CERO_WINDOWS
	windows
#else
	#error Undefined system.
#endif
{
	void initialize_system_state();
}

void initialize_system_state()
{
#ifdef CERO_WINDOWS
	windows::initialize_system_state();
#endif
}

} // namespace cero