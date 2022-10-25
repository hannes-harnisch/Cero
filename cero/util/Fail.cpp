#include "Fail.hpp"

void fail_unreachable()
{
	std::cout << "The compiler reached code that should be unreachable." << std::endl;
	std::abort();
}

void to_do()
{
	std::cout << "Not yet implemented." << std::endl;
	std::abort();
}
