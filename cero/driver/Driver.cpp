#include "Driver.hpp"
#include "driver/Config.hpp"
#include "driver/Source.hpp"
#include "syntax/Lexer.hpp"
#include "util/Fail.hpp"

static ExitCode perform_build(const Config& config)
{
	auto exit_code = ExitCode::Success;

	for (auto file_path : config.file_paths)
	{
		auto source = Source::from(file_path);
		if (!source.has_value())
			return ExitCode::NoInput;

		auto reporter = build_file(*source);
		if (reporter.has_reports())
			exit_code = ExitCode::DataError;
	}

	return exit_code;
}

static ExitCode perform_command(const Config& config)
{
	switch (config.command)
	{
		case Command::Build: return perform_build(config);
		case Command::Clean:
		case Command::Run:
		case Command::Eval: to_do();
	}
	return ExitCode::InternalError;
}

ExitCode run_driver(std::span<std::string_view> args)
{
	auto config = Config::from(args);

	if (!config.has_value())
		return config.error();

	return perform_command(*config);
}

Reporter build_file(const Source& source)
{
	Reporter reporter;

	auto tokens = lex(source, reporter);
	// TODO: add stages

	reporter.finalize(tokens);
	return reporter;
}
