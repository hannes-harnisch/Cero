#pragma once

#include "driver/ExitCode.hpp"

#include <expected>
#include <span>
#include <string_view>
#include <vector>

enum class Command
{
	None,
	Help,
	Build,
	Clean,
	Run,
	Eval,
	Invalid
};

struct Config
{
	static constexpr uint32_t DEFAULT_TAB_SIZE = 4;

	Command						  command = Command::None;
	std::vector<std::string_view> file_paths;

	uint32_t tab_size			= DEFAULT_TAB_SIZE;
	bool	 warnings_as_errors = false;

	Config() = default;
	Config(std::span<std::string_view> args);

private:
	void set_command(std::string_view arg);
	void set_option(std::string_view arg);
};
