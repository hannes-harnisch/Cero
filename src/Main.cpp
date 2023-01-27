#include "cero/driver/Initialize.hpp"
#include "driver/Command.hpp"
#include "driver/ExitCode.hpp"

namespace cero
{

ExitCode main(int argc, char* argv[])
{
	if (argc < 1)
		return ExitCode::Usage;

	initialize_system_state();

	std::vector<std::string_view> args(argv + 1, argv + argc);
	return run_command(args);
}

} // namespace cero

int main(int argc, char* argv[])
{
	return static_cast<int>(cero::main(argc, argv));
}
