#include "LexExhaustive.hpp"

#include "util/Test.hpp"

#include <cero/syntax/Lexer.hpp>

cero::TokenStream lex_exhaustive(const cero::Source& source)
{
	ExhaustiveReporter reporter(cero::Reporter(cero::Config()), source.get_path());
	return cero::lex(source, reporter);
}