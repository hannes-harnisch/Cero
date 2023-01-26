#include "cero/driver/SourceLocation.hpp"

namespace cero
{

std::string SourceLocation::to_string() const
{
	return std::format("{}:{}:{}", file, line, column);
}

} // namespace cero