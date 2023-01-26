#pragma once

#include <span>
#include <string_view>
#include <vector>

namespace cero
{

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
	bool	 log_tokens			= false;
	bool	 log_ast			= false;

	Config() = default;
	explicit Config(std::span<std::string_view> args);

private:
	void set_command(std::string_view arg);
	void set_option(std::string_view arg);
};

} // namespace cero