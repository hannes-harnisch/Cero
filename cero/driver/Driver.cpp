#include "Driver.hpp"

#include "driver/Build.hpp"
#include "driver/Config.hpp"
#include "driver/Source.hpp"
#include "util/Fail.hpp"

namespace cero
{

namespace
{
	void print_help()
	{}

	ExitCode perform_no_command()
	{
		std::cout << "No parameters specified." << std::endl;
		print_help();
		return ExitCode::Usage;
	}

	ExitCode perform_help_command()
	{
		print_help();
		return ExitCode::Success;
	}

	ExitCode perform_build_command(const Config& config)
	{
		auto exit_code = ExitCode::Success;

		for (auto file_path : config.file_paths)
		{
			auto source = Source::from_file(file_path, config);
			if (!source.has_value())
				return ExitCode::NoInput;

			auto reporter = build_source(*source, config);
			if (reporter.has_reports())
				exit_code = ExitCode::DataError;
		}

		return exit_code;
	}

	ExitCode perform_invalid_command()
	{
		std::cout << "Unknown command." << std::endl;
		print_help();
		return ExitCode::Usage;
	}

	ExitCode perform_command(const Config& config)
	{
		using enum Command;
		switch (config.command)
		{
			case None: return perform_no_command();
			case Help: return perform_help_command();
			case Build: return perform_build_command(config);
			case Clean:
			case Run:
			case Eval: to_do();
			case Invalid: return perform_invalid_command();
		}
		fail_unreachable();
	}
} // namespace

ExitCode run_driver(std::span<std::string_view> args)
{
	Config config(args);
	return perform_command(config);
}

void init_platform();

void initialize()
{
	init_platform();
}

} // namespace cero
