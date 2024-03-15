#include "CodeLocation.hpp"

namespace cero {

CodeLocation CodeLocation::blank(std::string_view source_name) {
	return CodeLocation {source_name, 0, 0};
}

std::string CodeLocation::to_string() const {
	return fmt::format("{}:{}:{}", source_name, line, column);
}

std::string CodeLocation::to_short_string() const {
	return fmt::format("[{}:{}]", line, column);
}

} // namespace cero
