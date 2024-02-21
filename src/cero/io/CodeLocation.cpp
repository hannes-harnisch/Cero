#include "CodeLocation.hpp"

namespace cero {

std::string CodeLocation::to_string() const {
	return fmt::format("{}:{}:{}", file, line, column);
}

} // namespace cero
