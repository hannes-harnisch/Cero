#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace cero {

enum class Command {
	Help,
	Version,
	Build,
	Install,
	Clean,
	Run,
};

struct Config {
	Command command = Command::Help;
	std::string_view path;
	uint8_t tab_size = DefaultTabSize;
	bool verbose = false;
	bool warnings_as_errors = false;
	bool print_tokens = false;
	bool print_ast = false;

	static std::optional<Config> from(std::span<char*> args);

	static constexpr uint8_t DefaultTabSize = 4;

private:
	bool parse_command(std::string_view arg);
	bool parse_option(std::string_view arg);

	bool parse_tab_size(std::string_view arg);
};

} // namespace cero
