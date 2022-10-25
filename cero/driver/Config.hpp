#pragma once

#include "driver/ExitCode.hpp"

#include <expected>
#include <span>
#include <string_view>
#include <vector>

enum class Command
{
	Build,
	Clean,
	Run,
	Eval
};

class Option
{};

struct Config
{
	Command						  command;
	std::vector<std::string_view> file_paths;
	std::vector<Option>			  options;

	static std::expected<Config, ExitCode> from(std::span<std::string_view> args);
};
