#pragma once

#include <cero/driver/Source.hpp>

class TestSource : public Source
{
public:
	TestSource(std::string source, std::source_location loc = std::source_location::current());
};
