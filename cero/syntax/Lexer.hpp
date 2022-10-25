#pragma once

#include "driver/Source.hpp"
#include "syntax/TokenStream.hpp"

TokenStream lex(const Source& source, Reporter& reporter);
