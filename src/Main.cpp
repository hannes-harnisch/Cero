#include "cero/driver/Run.hpp"

int main(int argc, char* argv[]) {
	if (argc < 1) {
		return EXIT_FAILURE;
	}

	std::span<char*> args(argv + 1, argv + argc);
	bool succeeded = cero::run(args);
	if (succeeded) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}
