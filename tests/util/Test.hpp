#pragma once

#include <doctest/doctest.h>

#define TEST(name) DOCTEST_CREATE_AND_REGISTER_FUNCTION(Test_##name, #name)
