#pragma once

#include <cero/io/Config.hpp>
#include <cero/io/Reporter.hpp>
#include <cero/io/Source.hpp>
#include <doctest/doctest.h>

#include <string_view>

cero::SourceLock make_test_source(std::string_view source_text, const cero::Config& config = cero::Config());

void build_test_source(cero::Reporter& reporter, std::string_view source_text);

// Creates and registers a test case.
#define CERO_TEST(NAME) DOCTEST_CREATE_AND_REGISTER_FUNCTION(NAME, #NAME)
