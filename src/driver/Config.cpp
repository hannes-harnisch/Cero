#include "Config.hpp"

namespace cero {

Config::Config(std::span<std::string_view> args) {
	std::queue args_queue(std::deque(args.begin(), args.end()));

	if (!args_queue.empty()) {
		set_command(args_queue.front());
		args_queue.pop();
	}

	while (!args_queue.empty()) {
		set_option(args_queue.front());
		args_queue.pop();
	}
}

void Config::set_command(std::string_view arg) {
	using enum Command;

	if (arg == "help" || arg == "--help" || arg == "-h")
		command = Help;
	else if (arg == "build")
		command = Build;
	else if (arg == "install")
		command = Install;
	else if (arg == "clean")
		command = Clean;
	else if (arg == "run")
		command = Run;
	else if (arg == "eval")
		command = Eval;
	else
		command = Unknown;
}

void Config::set_option(std::string_view arg) {
	if (arg == "-Werror")
		warnings_as_errors = true;
	else if (arg == "--log-tokens")
		log_tokens = true;
	else if (arg == "--log-ast")
		log_ast = true;
	// check for all other options here in the future
	else
		path = arg;
}

} // namespace cero
