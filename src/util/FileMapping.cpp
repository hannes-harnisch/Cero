#include "FileMapping.hpp"

namespace cero {

std::string_view FileMapping::get_text() const {
	if (size == 0)
		return "";

	const char* str = static_cast<const char*>(map_addr);
	return std::string_view(str, size);
}

size_t FileMapping::get_size() const {
	return size;
}

FileMapping& FileMapping::operator=(FileMapping&& other) noexcept {
	this->~FileMapping();
	new (this) FileMapping(std::move(other));
	return *this;
}

} // namespace cero