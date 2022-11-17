#include "Driver.hpp"

#include "driver/Config.hpp"
#include "driver/Source.hpp"
#include "syntax/Lexer.hpp"
#include "syntax/Parser.hpp"
#include "util/Fail.hpp"

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
		auto exit_code = ExitCode::Success;

		for (auto file_path : config.file_paths)
		{
			auto source = Source::from_file(file_path, config);
			if (!source.has_value())
				return ExitCode::NoInput;

			auto reporter = build_file(*source);
			if (reporter.has_reports())
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

	ExitCode perform_command(const Config& config)
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

ExitCode run_driver(std::span<std::string_view> args)
{
	Config config(args);
	return perform_command(config);
}

Reporter build_file(const Source& source)
{
	Reporter reporter;

	auto tokens = lex(source, reporter);
	auto ast	= parse(tokens, source, reporter);

	return reporter;
}
