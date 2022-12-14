#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

void init_platform();

int main(int argc, char* argv[])
{
	init_platform();

	doctest::Context context;
	context.applyCommandLine(argc, argv);
	return context.run();
}
