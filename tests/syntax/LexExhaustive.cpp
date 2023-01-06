#include "LexExhaustive.hpp"

#include "util/Test.hpp"

#include <cero/syntax/Lexer.hpp>

TokenStream lex_exhaustive(const Source& source)
{
	ExhaustiveReporter reporter(Reporter(Config()), source.get_path());
	return lex(source, reporter);
}