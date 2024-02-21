#pragma once

#include "cero/io/Reporter.hpp"
#include "cero/io/Source.hpp"
#include "cero/syntax/Ast.hpp"
#include "cero/syntax/TokenStream.hpp"

namespace cero {

Ast parse(const SourceLock& source, Reporter& reporter);
Ast parse(const TokenStream& token_stream, const SourceLock& source, Reporter& reporter);

} // namespace cero