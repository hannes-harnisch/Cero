#pragma once

#include "driver/Reporter.hpp"
#include "driver/Source.hpp"
#include "syntax/TokenStream.hpp"

TokenStream lex(const Source& source, Reporter& reporter);
