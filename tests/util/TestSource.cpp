#include "TestSource.hpp"

TestSource::TestSource(std::string source, std::source_location loc) :
	Source(std::move(source), loc.function_name())
{}
