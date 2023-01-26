#pragma once

#include "cero/driver/Reporter.hpp"
#include "cero/driver/Source.hpp"
#include "cero/syntax/TokenStream.hpp"

namespace cero
{

TokenStream lex(const Source& source, Reporter& reporter);

}