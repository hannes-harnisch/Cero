#include "CodeLocation.hpp"

namespace cero {

std::string CodeLocation::to_string() const {
	return std::format("{}:{}:{}", file, line, column);
}

} // namespace cero
