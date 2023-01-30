#pragma once

#include <cero/driver/Config.hpp>
#include <cero/driver/Reporter.hpp>
#include <cero/driver/Source.hpp>
#include <doctest/doctest.h>

#include <source_location>
#include <string>

cero::Source make_test_source(std::string		   source_text,
							  std::source_location location = std::source_location::current(),
							  const cero::Config&  config	= cero::Config());

void build_test_source(cero::Reporter&		reporter,
					   std::string			source_text,
					   std::source_location location = std::source_location::current());

// Defines a test case function by combining the test name with an easily identifiable prefix.
#define CERO_TEST(name) DOCTEST_CREATE_AND_REGISTER_FUNCTION(Test_##name, #name)
