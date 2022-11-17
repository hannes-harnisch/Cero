#include "Fail.hpp"

#include "driver/ExitCode.hpp"

namespace
{
	[[noreturn]] void exit(ExitCode exit_code)
	{
		std::exit(static_cast<int>(exit_code));
	}
} // namespace

void fail_unreachable()
{
	std::cout << "The compiler reached code that should be unreachable." << std::endl;
	exit(ExitCode::InternalError);
}

void to_do(std::source_location location)
{
	std::cout << "Not yet implemented." << std::endl;
	std::cout << "\tFile:\t\t" << location.file_name() << std::endl;
	std::cout << "\tFunction:\t" << location.function_name() << std::endl;
	exit(ExitCode::InternalError);
}
