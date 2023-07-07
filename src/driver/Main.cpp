#include "driver/Compiler.hpp"

int main(int argc, char* argv[]) {
	if (argc < 1)
		return EXIT_FAILURE;

	std::vector<std::string_view> args(argv + 1, argv + argc);
	return cero::run_compiler(args) ? EXIT_SUCCESS : EXIT_FAILURE;
}