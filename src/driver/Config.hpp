#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace cero {

enum class Command {
	None,
	Help,
	Build,
	Install,
	Clean,
	Run,
	Eval,
	Unknown
};

constexpr inline uint32_t DefaultTabSize = 4;

struct Config {
	Command			 command = Command::None;
	std::string_view path;
	uint32_t		 tab_size			= DefaultTabSize;
	bool			 warnings_as_errors = false;
	bool			 log_tokens			= false;
	bool			 log_ast			= false;

	Config() = default;
	explicit Config(std::span<std::string_view> args);

private:
	void set_command(std::string_view arg);
	void set_option(std::string_view arg);
};

} // namespace cero
