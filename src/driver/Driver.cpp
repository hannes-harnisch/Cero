#include "Driver.hpp"
#include "driver/Config.hpp"

static ExitCode perform_build(const Config& config)
{
	return ExitCode::InternalError;
}

static ExitCode run_driver_with_config(const Config& config)
{
	switch (config.command)
	{
		case Command::Build: return perform_build(config);
		case Command::Clean:
		case Command::Run:
		case Command::Eval: std::abort();
	}
	return ExitCode::InternalError;
}

ExitCode run_driver(std::vector<std::string_view> args)
{
	auto config = Config::from(args);

	if (!config.has_value())
		return config.error();

	return run_driver_with_config(*config);
}
