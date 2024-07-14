#include "BuildCommand.hpp"

#include "cero/io/ConsoleReporter.hpp"
#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Parse.hpp"
#include "cero/util/SystemError.hpp"

namespace cero {

bool run_build_command(const Configuration& config) {
	auto source = Source::from_file(config.path, config);

	ConsoleReporter reporter(config);
	build_source(source, config, reporter);

	return !reporter.has_errors();
}

static void build_locked_source(const SourceGuard& source, const Configuration& config, Reporter& reporter) {
	if (config.print_source) {
		fmt::println("{}", source.get_text());
	}

	auto token_stream = lex(source, reporter, false);
	if (config.print_tokens) {
		fmt::println("{}", token_stream.to_string(source));
	}

	auto ast = parse(token_stream, source, reporter);
	if (config.print_ast) {
		fmt::println("{}", ast.to_string(source));
	}
}

void build_source(const Source& source, const Configuration& config, Reporter& reporter) {
	auto lock_result = source.lock();
	if (auto locked_source = lock_result.value()) {
		build_locked_source(*locked_source, config, reporter);
	} else {
		auto& error = *lock_result.error();
		const auto error_code = static_cast<std::errc>(error.value());

		const auto blank = CodeLocation::blank(source.get_name());
		if (error_code == std::errc::no_such_file_or_directory) {
			reporter.report(Message::FileNotFound, blank, {});
		} else {
			reporter.report(Message::CouldNotOpenFile, blank, MessageArgs(get_system_error_message(error)));
		}
	}
}

} // namespace cero
