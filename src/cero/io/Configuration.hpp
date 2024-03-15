#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace cero {

/// The primary command line argument for the compiler for a given execution, that determines what it should mainly do when
/// executed.
enum class Command : uint8_t {
	Help,
	Version,
	Build,
	Install,
	Clean,
	Run,
};

/// Holds all the values that decide the compiler's customizable behavior for a given execution, usually parsed from command
/// line arguments. Should be passed into every function that does something that can be influenced by command line arguments.
struct Configuration {
	/// What the compiler should mainly do for a given execution.
	Command command = Command::Help;

	/// Temporary, before a proper build system exists. Holds the path of the file to compile.
	std::string_view path;

	/// The tab size of the source code as intended by the author, to make the locations in diagnostic messages accurate.
	uint8_t tab_size = DefaultTabSize;

	/// Decides whether verbose output is enabled.
	bool verbose = false;

	/// Decides whether warnings should be treated as errors.
	bool warnings_as_errors = false;

	/// Decides whether the compiler should print the source before lexing.
	bool print_source = false;

	/// Decides whether the compiler should print the token stream after lexing.
	bool print_tokens = false;

	/// Decides whether the compiler should print the AST after parsing.
	bool print_ast = false;

	/// Create a configuration from command line arguments.
	static std::optional<Configuration> from(std::span<char*> args);

	static constexpr uint8_t DefaultTabSize = 4;

private:
	bool parse_command(std::string_view arg);
	bool parse_option(std::string_view arg);

	bool parse_tab_size(std::string_view arg);
};

} // namespace cero
