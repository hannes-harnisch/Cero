#include "Run.hpp"

#include "cero/driver/BuildCommand.hpp"
#include "cero/io/Config.hpp"
#include "cero/util/Fail.hpp"

namespace cero {

namespace {

	bool run_help_command() {
		to_do();
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

	bool run_eval_command() {
		to_do();
	}

} // namespace

bool run(std::span<char*> args) {
	if (auto config = Config::from(args)) {
		switch (config->command) {
			using enum Command;
			case None:
			case Help: return run_help_command();
			case Build: return run_build_command(*config);
			case Install: return run_install_command();
			case Clean: return run_clean_command();
			case Run: return run_run_command();
			case Eval: return run_eval_command();
		}
		fail_unreachable();
	}
	return false;
}

} // namespace cero
