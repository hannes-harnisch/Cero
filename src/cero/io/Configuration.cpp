#include "Configuration.hpp"

namespace cero {

namespace {

	std::string_view get_arg_value_string(std::string_view arg) {
		const size_t i = arg.find('=');
		return arg.substr(i + 1);
	}

} // namespace

std::optional<Configuration> Configuration::from(std::span<char*> args) {
	Configuration config;

	if (!args.empty()) {
		if (!config.parse_command(args[0])) {
			return std::nullopt;
		}
	}

	for (size_t i = 1; i < args.size(); ++i) {
		if (!config.parse_option(args[i])) {
			return std::nullopt;
		}
	}

	return config;
}

bool Configuration::parse_command(std::string_view arg) {
	if (arg == "-h" || arg == "--help") {
		command = Command::Help;
	} else if (arg == "-V" || arg == "--version") {
		command = Command::Version;
	} else if (arg == "build") {
		command = Command::Build;
	} else if (arg == "install") {
		command = Command::Install;
	} else if (arg == "clean") {
		command = Command::Clean;
	} else if (arg == "run") {
		command = Command::Run;
	} else {
		fmt::println("'{}' is not a valid command. See 'cero --help'.", arg);
		return false;
	}

	return true;
}

bool Configuration::parse_option(std::string_view arg) {
	if (arg.starts_with("--tab-size=")) {
		return parse_tab_size(arg);
	}
	// check for all other value-based options here in the future

	if (arg == "-v" || arg == "--verbose") {
		verbose = true;
	} else if (arg == "-Werror") {
		warnings_as_errors = true;
	} else if (arg == "--print-source") {
		print_source = true;
	} else if (arg == "--print-tokens") {
		print_tokens = true;
	} else if (arg == "--print-ast") {
		print_ast = true;
	}
	// check for all other boolean options here in the future
	else {
		path = arg;
	}

	return true;
}

bool Configuration::parse_tab_size(std::string_view arg) {
	auto str = get_arg_value_string(arg);

	uint8_t tab_size_value;
	auto result = std::from_chars(str.data(), str.data() + str.size(), tab_size_value);
	if (result.ec == std::errc()) {
		tab_size = tab_size_value;
		return true;
	} else {
		fmt::println("--tab-size must be specified with a value between 0 and 255.");
		return false;
	}
}

} // namespace cero
