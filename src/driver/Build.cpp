#include "cero/driver/Build.hpp"

#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Parse.hpp"

namespace cero
{

void build_source(const Source& source, const Config& config, Reporter& reporter)
{
	auto stream = lex(source, reporter);
	if (config.log_tokens)
		stream.log(source);

	auto ast = parse(stream, source, reporter);
	if (config.log_ast)
		ast.log(source);
}

} // namespace cero