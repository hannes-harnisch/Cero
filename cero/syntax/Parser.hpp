#pragma once

#include "driver/Reporter.hpp"
#include "syntax/TokenStream.hpp"

void parse(const TokenStream& tokens, Reporter& reporter);
