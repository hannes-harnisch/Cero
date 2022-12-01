#pragma once

#include "driver/Reporter.hpp"
#include "driver/Source.hpp"
#include "syntax/SyntaxTree.hpp"
#include "syntax/TokenStream.hpp"

SyntaxTree parse(const TokenStream& token_stream, const Source& source, Reporter& reporter);
