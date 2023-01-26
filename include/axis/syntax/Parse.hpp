#pragma once

#include "cero/driver/Reporter.hpp"
#include "cero/driver/Source.hpp"
#include "cero/syntax/SyntaxTree.hpp"
#include "cero/syntax/TokenStream.hpp"

namespace cero
{

SyntaxTree parse(const Source& source, Reporter& reporter);
SyntaxTree parse(const TokenStream& token_stream, const Source& source, Reporter& reporter);

} // namespace cero