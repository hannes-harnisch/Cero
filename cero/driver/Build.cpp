#include "Build.hpp"

#include "syntax/Lexer.hpp"
#include "syntax/Parser.hpp"

Reporter build_source(const Source& source, const Config& config)
{
	Reporter reporter;

	auto stream = lex(source, reporter);
	if (config.log_tokens)
		stream.log(source);

	auto ast = parse(stream, source, reporter);
	if (config.log_ast)
		ast.log(source);

	return reporter;
}