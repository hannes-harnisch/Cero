#include "FileMapping.hpp"

namespace cero {

std::string_view FileMapping::get_text() const {
	const char* str = static_cast<const char*>(addr_);
	return std::string_view(str, size_);
}

size_t FileMapping::get_size() const {
	return size_;
}

FileMapping& FileMapping::operator=(FileMapping&& other) noexcept {
	this->~FileMapping();
	new (this) FileMapping(std::move(other));
	return *this;
}

} // namespace cero
