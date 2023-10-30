#include "BuildCommand.hpp"

#include "driver/ConsoleReporter.hpp"
#include "syntax/Lex.hpp"
#include "syntax/Parse.hpp"

namespace cero {

void build_source(const Source& source, const Config& config, Reporter& reporter) {
	auto token_stream = lex(source, reporter);
	if (config.log_tokens) {
		token_stream.log(source);
	}

	auto ast = parse(token_stream, source, reporter);
	if (config.log_ast) {
		ast.log(source);
	}
}

bool on_build_command(const Config& config) {
	ConsoleReporter reporter(config);
	auto source = Source::from_file(config.path, config);

	build_source(source, config, reporter);

	return !reporter.has_errors();
}

} // namespace cero
