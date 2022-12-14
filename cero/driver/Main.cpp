#include "driver/Driver.hpp"
#include "driver/ExitCode.hpp"

void init_platform();

int main(int argc, char* argv[])
{
	ExitCode exit_code;
	if (argc < 1)
		exit_code = ExitCode::Usage;

	init_platform();

	std::vector<std::string_view> args(argv + 1, argv + argc);
	exit_code = run_driver(args);

	return static_cast<int>(exit_code);
}
