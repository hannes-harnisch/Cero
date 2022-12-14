#include "SourceLocation.hpp"

std::string SourceLocation::to_string() const
{
	return std::format("{}:{}:{}", file, line, column);
}
