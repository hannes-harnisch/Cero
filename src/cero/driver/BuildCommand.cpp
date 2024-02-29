#include "BuildCommand.hpp"

#include "cero/io/ConsoleReporter.hpp"
#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Parse.hpp"

namespace cero {

bool run_build_command(const Config& config) {
	auto source = Source::from_file(config.path, config);

	ConsoleReporter reporter(config);
	build_source(source, config, reporter);

	return !reporter.has_errors();
}

namespace {

	void build_locked_source(const LockedSource& source, const Config& config, Reporter& reporter) {
		auto token_stream = lex(source, reporter);
		if (config.print_tokens) {
			fmt::println("{}", token_stream.to_string(source));
		}

		auto ast = parse(token_stream, source, reporter);
		if (config.print_ast) {
			fmt::println("{}", ast.to_string(source));
		}
	}

} // namespace

void build_source(const Source& source, const Config& config, Reporter& reporter) {
	auto lock_result = source.lock();
	if (auto locked_source = lock_result.value()) {
		build_locked_source(*locked_source, config, reporter);
	} else {
		auto& error = *lock_result.error();

		const int error_code = error.value();
		const CodeLocation blank_location {source.get_path(), 0, 0};
		if (static_cast<std::errc>(error_code) == std::errc::no_such_file_or_directory) {
			reporter.report(Message::FileNotFound, blank_location, {});
		} else {
			reporter.report(Message::CouldNotOpenFile, blank_location, ReportArgs(error_code));
		}
	}
}

} // namespace cero
