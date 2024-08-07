#pragma once

#include "cero/io/Reporter.hpp"
#include "cero/io/Source.hpp"
#include "cero/syntax/TokenStream.hpp"

namespace cero {

TokenStream lex(const SourceGuard& source, Reporter& reporter, bool lex_comments);

}
