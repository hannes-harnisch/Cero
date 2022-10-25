#include "Config.hpp"

static void print_help()
{
	std::cout << "No parameters specified."; // TODO: Print args info here.
}

static std::optional<Command> read_command(std::string_view arg)
{
	if (arg == "build")
		return Command::Build;
	if (arg == "clean")
		return Command::Clean;
	if (arg == "run")
		return Command::Run;
	if (arg == "eval")
		return Command::Eval;

	return {};
}

static bool is_file_path(std::string_view arg)
{
	return !arg.starts_with("-"); // TODO
}

static std::optional<Option> read_option(std::string_view)
{
	return {};
}

std::expected<Config, ExitCode> Config::from(std::span<std::string_view> args)
{
	if (args.empty())
	{
		print_help();
		return std::unexpected(ExitCode::Usage);
	}

	std::deque deque(args.begin(), args.end());
	std::queue args_queue(deque);

	auto first = args_queue.front();
	if (first == "help" || first == "--help" || first == "-h")
	{
		print_help();
		return std::unexpected(ExitCode::Success);
	}

	auto cmd = read_command(first);
	args_queue.pop();
	if (!cmd.has_value())
	{
		std::cout << "Unknown command." << std::endl;
		print_help();
		return std::unexpected(ExitCode::Usage);
	}

	Config config {.command = *cmd};

	while (!args_queue.empty())
	{
		while (true)
		{
			auto arg = args_queue.front();
			if (!is_file_path(arg))
				break;

			config.file_paths.emplace_back(arg);
			args_queue.pop();
		}
		while (true)
		{
			auto option = read_option(args_queue.front());
			if (!option.has_value())
				break;

			config.options.emplace_back(std::move(*option));
			args_queue.pop();
		}
	}
	return config;
}
