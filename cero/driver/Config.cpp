#include "Config.hpp"

Config::Config(std::span<std::string_view> args)
{
	std::queue args_queue(std::deque(args.begin(), args.end()));

	if (!args_queue.empty())
	{
		set_command(args_queue.front());
		args_queue.pop();
	}

	while (!args_queue.empty())
	{
		set_option(args_queue.front());
		args_queue.pop();
	}
}

void Config::set_command(std::string_view arg)
{
	using enum Command;

	if (arg == "help" || arg == "--help" || arg == "-h")
		command = Help;
	else if (arg == "build")
		command = Build;
	else if (arg == "clean")
		command = Clean;
	else if (arg == "run")
		command = Run;
	else if (arg == "eval")
		command = Eval;
	else
		command = Invalid;
}

namespace
{
	bool is_file_path(std::string_view arg)
	{
		// TODO: this just treats everything as an input file path that doesn't start with a dash...
		return !arg.starts_with("-");
	}
} // namespace

void Config::set_option(std::string_view arg)
{
	if (is_file_path(arg))
		file_paths.emplace_back(arg);

	// check for all other options here in the future
}
