#include "ParseExhaustive.hpp"

#include "util/Test.hpp"

#include <cero/syntax/Lexer.hpp>
#include <cero/syntax/Parser.hpp>

SyntaxTree parse_exhaustive(const Source& source)
{
	ExhaustiveReporter reporter(source.get_path());

	auto stream = lex(source, reporter);
	return parse(stream, source, reporter);
}
