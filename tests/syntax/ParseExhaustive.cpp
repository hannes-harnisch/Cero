#include "ParseExhaustive.hpp"

#include "util/Test.hpp"

#include <cero/syntax/Lexer.hpp>
#include <cero/syntax/Parser.hpp>

cero::SyntaxTree parse_exhaustive(const cero::Source& source)
{
	ExhaustiveReporter reporter(cero::Reporter(cero::Config()), source.get_path());

	auto stream = cero::lex(source, reporter);
	return cero::parse(stream, source, reporter);
}
