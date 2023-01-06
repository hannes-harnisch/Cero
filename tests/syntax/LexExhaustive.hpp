#pragma once

#include <cero/driver/Source.hpp>
#include <cero/syntax/TokenStream.hpp>

cero::TokenStream lex_exhaustive(const cero::Source& source);
