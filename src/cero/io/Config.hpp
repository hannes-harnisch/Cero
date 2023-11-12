#pragma once

#include <cstdint>
#include <optional>
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
};

struct Config {
	Command command = Command::None;
	std::string_view path;
	uint8_t tab_size = DefaultTabSize;
	bool warnings_as_errors = false;
	bool log_tokens = false;
	bool log_ast = false;

	static std::optional<Config> from(std::span<char*> args);

	static constexpr uint8_t DefaultTabSize = 4;

private:
	bool parse_command(std::string_view arg);
	bool parse_option(std::string_view arg);

	bool parse_tab_size(std::string_view arg);
};

} // namespace cero
