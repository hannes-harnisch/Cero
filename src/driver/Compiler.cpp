#include "Compiler.hpp"

#include "driver/BuildCommand.hpp"
#include "driver/Config.hpp"
#include "util/Fail.hpp"

namespace cero {

namespace {

	void print_help() {
	}

	bool on_no_command() {
		std::cout << "No command specified." << std::endl;
		print_help();
		return true;
	}

	bool on_help_command() {
		print_help();
		return true;
	}

	bool on_install_command() {
		to_do();
	}

	bool on_clean_command() {
		to_do();
	}

	bool on_run_command() {
		to_do();
	}

	bool on_eval_command() {
		to_do();
	}

	bool on_unknown_command() {
		std::cout << "Unknown command." << std::endl;
		print_help();
		return false;
	}

} // namespace

bool run_compiler(std::span<std::string_view> args) {
	Config config(args);

	using enum Command;
	switch (config.command) {
		case None: return on_no_command();
		case Help: return on_help_command();
		case Build: return on_build_command(config);
		case Install: return on_install_command();
		case Clean: return on_clean_command();
		case Run: return on_run_command();
		case Eval: return on_eval_command();
		case Unknown: return on_unknown_command();
	}
	fail_unreachable();
}

} // namespace cero
