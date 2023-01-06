#pragma once

#include <cero/driver/Source.hpp>
#include <cero/syntax/TokenStream.hpp>

TokenStream lex_exhaustive(const Source& source);
