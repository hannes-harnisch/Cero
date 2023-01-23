#include "driver/Driver.hpp"
#include "driver/ExitCode.hpp"

namespace cero
{

ExitCode main(int argc, char* argv[])
{
	if (argc < 1)
		return ExitCode::Usage;

	initialize();

	std::vector<std::string_view> args(argv + 1, argv + argc);
	return run_driver(args);
}

} // namespace cero

int main(int argc, char* argv[])
{
	return static_cast<int>(cero::main(argc, argv));
}
