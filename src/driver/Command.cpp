#include "Command.hpp"

#include "cero/driver/Build.hpp"
#include "cero/driver/Config.hpp"
#include "cero/driver/Source.hpp"
#include "driver/ConsoleReporter.hpp"
#include "util/Fail.hpp"

namespace cero
{

namespace
{
	void print_help()
	{}

	ExitCode on_no_command()
	{
		std::cout << "No parameters specified." << std::endl;
		print_help();
		return ExitCode::Usage;
	}

	ExitCode on_help_command()
	{
		print_help();
		return ExitCode::Success;
	}

	ExitCode on_build_command(const Config& config)
	{
		ConsoleReporter reporter(config);

		auto exit_code = ExitCode::Success;
		for (auto file_path : config.file_paths)
		{
			auto source = Source::from_file(file_path, config);
			if (!source.has_value())
				return ExitCode::NoInput;

			build_source(*source, config, reporter);
			if (reporter.has_errors())
				exit_code = ExitCode::DataError;
		}

		return exit_code;
	}

	ExitCode on_invalid_command()
	{
		std::cout << "Unknown command." << std::endl;
		print_help();
		return ExitCode::Usage;
	}

	ExitCode dispatch_command(const Config& config)
	{
		using enum Command;
		switch (config.command)
		{
			case None: return on_no_command();
			case Help: return on_help_command();
			case Build: return on_build_command(config);
			case Clean:
			case Run:
			case Eval: to_do();
			case Invalid: return on_invalid_command();
		}
		fail_unreachable();
	}
} // namespace

ExitCode run_command(std::span<std::string_view> args)
{
	Config config(args);
	return dispatch_command(config);
}

} // namespace cero
