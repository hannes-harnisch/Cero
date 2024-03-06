#pragma once

#include <cero/io/Configuration.hpp>
#include <cero/io/Reporter.hpp>
#include <cero/io/Source.hpp>
#include <doctest/doctest.h>

#include <string_view>

namespace tests {

cero::SourceGuard make_test_source(std::string_view source_text, const cero::Configuration& config = cero::Configuration());

void build_test_source(cero::Reporter& reporter, std::string_view source_text);

std::string_view get_current_test_name();

/// Creates and registers a test case.
#define CERO_TEST(NAME) DOCTEST_CREATE_AND_REGISTER_FUNCTION(NAME, #NAME)

} // namespace tests
