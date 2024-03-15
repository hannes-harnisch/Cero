#include "Run.hpp"

#include "cero/driver/BuildCommand.hpp"
#include "cero/driver/Environment.hpp"
#include "cero/driver/Version.hpp"
#include "cero/io/Configuration.hpp"
#include "cero/util/Fail.hpp"

namespace cero {

namespace {

	bool run_help_command() {
		static constexpr auto help = R"_____(
Cero compiler, version {}.{}.{}

usage: cero [COMMAND] [OPTIONS]

commands:
    build               Build the current package
    run                 Build and run the current package
    clean               Remove all build artifacts for the current package
    install             Build and install the current package

options:
    -h, --help          Show this message
    -v, --verbose       Give verbose output
    -V, --version       Show version and build info for the compiler
)_____";

		fmt::println(help, version::Major, version::Minor, version::Patch);
		return true;
	}

	bool run_version_command() {
		fmt::println("Cero compiler, version {}.{}.{}", version::Major, version::Minor, version::Patch);
		return true;
	}

	bool run_install_command() {
		to_do();
	}

	bool run_clean_command() {
		to_do();
	}

	bool run_run_command() {
		to_do();
	}

} // namespace

bool run(std::span<char*> args) {
	initialize_environment();

	if (auto config = Configuration::from(args)) {
		switch (config->command) {
			using enum Command;
			case Help:	  return run_help_command();
			case Version: return run_version_command();
			case Build:	  return run_build_command(*config);
			case Install: return run_install_command();
			case Clean:	  return run_clean_command();
			case Run:	  return run_run_command();
		}
		fail_unreachable();
	}
	return false;
}

} // namespace cero
