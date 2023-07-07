#pragma once

#include "driver/Reporter.hpp"
#include "driver/Source.hpp"
#include "syntax/Ast.hpp"
#include "syntax/TokenStream.hpp"

namespace cero {

Ast parse(const Source& source, Reporter& reporter);
Ast parse(const TokenStream& token_stream, const Source& source, Reporter& reporter);

} // namespace cero