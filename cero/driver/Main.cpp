#include "driver/Driver.hpp"
#include "driver/ExitCode.hpp"

namespace
{
	ExitCode run(int argc, char* argv[])
	{
		if (argc < 1)
			return ExitCode::Usage;

		std::vector<std::string_view> args(argv + 1, argv + argc);
		return run_driver(args);
	}
} // namespace

int main(int argc, char* argv[])
{
	return static_cast<int>(run(argc, argv));
}
