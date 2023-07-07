#include "MappedFile.hpp"

namespace cero {

std::string_view MappedFile::get_text() const {
	if (size == 0)
		return "";

	const char* str = static_cast<const char*>(addr);
	return std::string_view(str, size);
}

size_t MappedFile::get_size() const {
	return size;
}

MappedFile& MappedFile::operator=(MappedFile&& other) noexcept {
	this->~MappedFile();
	new (this) MappedFile(std::move(other));
	return *this;
}

MappedFile::MappedFile(void* file, void* mapping, void* addr, size_t size) :
	file(file),
	mapping(mapping),
	addr(addr),
	size(size) {
}

} // namespace cero