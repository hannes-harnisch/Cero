#include "Config.hpp"

namespace cero {

namespace {

	std::string_view get_arg_value_string(std::string_view arg) {
		const size_t i = arg.find('=');
		return arg.substr(i + 1);
	}

} // namespace

std::optional<Config> Config::from(std::span<char*> args) {
	Config config;

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

bool Config::parse_command(std::string_view arg) {
	if (arg == "help" || arg == "--help" || arg == "-h") {
		command = Command::Help;
	} else if (arg == "build") {
		command = Command::Build;
	} else if (arg == "install") {
		command = Command::Install;
	} else if (arg == "clean") {
		command = Command::Clean;
	} else if (arg == "run") {
		command = Command::Run;
	} else if (arg == "eval") {
		command = Command::Eval;
	} else {
		std::cout << '\'' << arg << "' is not a valid command. See 'cero help'.\n";
		return false;
	}

	return true;
}

bool Config::parse_option(std::string_view arg) {
	if (arg.starts_with("--tab-size=")) {
		return parse_tab_size(arg);
	}

	if (arg == "-Werror") {
		warnings_as_errors = true;
	} else if (arg == "--log-tokens") {
		log_tokens = true;
	} else if (arg == "--log-ast") {
		log_ast = true;
	}
	// check for all other options here in the future
	else {
		path = arg;
	}

	return true;
}

bool Config::parse_tab_size(std::string_view arg) {
	auto str = get_arg_value_string(arg);

	uint8_t tab_size_value;
	auto result = std::from_chars(str.data(), str.data() + str.size(), tab_size_value);
	if (result.ec == std::errc()) {
		tab_size = tab_size_value;
		return true;
	} else {
		std::cout << "--tab-size must be specified with a value between 0 and 255.\n";
		return false;
	}
}

} // namespace cero
