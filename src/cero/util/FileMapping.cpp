#include "FileMapping.hpp"

namespace cero {

std::string_view FileMapping::get_text() const {
	return std::string_view(static_cast<const char*>(addr_), size_);
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
