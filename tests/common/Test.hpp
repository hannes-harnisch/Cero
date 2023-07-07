#pragma once

#include <doctest/doctest.h>
#include <driver/Config.hpp>
#include <driver/Reporter.hpp>
#include <driver/Source.hpp>

#include <source_location>
#include <string>

cero::Source make_test_source(std::string_view	   source_text,
							  std::source_location location = std::source_location::current(),
							  const cero::Config&  config	= cero::Config());

void build_test_source(cero::Reporter&		reporter,
					   std::string_view		source_text,
					   std::source_location location = std::source_location::current());

// Defines a test case function by combining the test name with an easily identifiable prefix.
#define CERO_TEST(name) DOCTEST_CREATE_AND_REGISTER_FUNCTION(Test_##name, #name)
