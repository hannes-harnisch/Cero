#include "Driver.hpp"

namespace cero
{
	int run(std::span<std::string_view> args)
	{
		cero::Driver driver;
		return driver.run_command(args) ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	Driver::Driver()
	{}

	Driver::Driver(std::streambuf* output_stream, std::streambuf* error_stream)
	{
		std::cout.rdbuf(output_stream);
		std::cerr.rdbuf(error_stream);
	}

	bool Driver::run_command(std::span<std::string_view> args)
	{
		return false;
	}
}
