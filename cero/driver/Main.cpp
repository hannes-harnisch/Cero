#include "driver/Driver.hpp"
#include "driver/ExitCode.hpp"

namespace cero
{

void init_platform();

}

int main(int argc, char* argv[])
{
	cero::ExitCode exit_code;
	if (argc < 1)
		exit_code = cero::ExitCode::Usage;

	cero::init_platform();

	std::vector<std::string_view> args(argv + 1, argv + argc);
	exit_code = cero::run_driver(args);

	return static_cast<int>(exit_code);
}
