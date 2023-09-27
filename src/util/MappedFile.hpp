#pragma once

#include <expected>
#include <string_view>
#include <system_error>

namespace cero {

class MappedFile {
public:
	static std::expected<MappedFile, std::error_code> from(std::string_view path);

	std::string_view get_text() const;
	size_t get_size() const;

	~MappedFile();
	MappedFile(MappedFile&& other) noexcept;
	MappedFile& operator=(MappedFile&& other) noexcept;

private:
	void* file;
	void* mapping;
	void* addr;
	size_t size;

	MappedFile(void* file, void* mapping, void* addr, size_t size);
};

} // namespace cero
