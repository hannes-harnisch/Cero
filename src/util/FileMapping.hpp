#pragma once

#include <expected>
#include <string_view>
#include <system_error>

namespace cero {

class FileMapping {
public:
	static std::expected<FileMapping, std::error_code> from(std::string_view path);

	std::string_view get_text() const;
	size_t get_size() const;

	~FileMapping();

	FileMapping(FileMapping&&) noexcept;
	FileMapping& operator=(FileMapping&&) noexcept;

private:
	size_t size;

#if defined(CERO_WINDOWS)
	void* file;
	void* mapping;
	const void* addr;
#elif defined(CERO_UNIX)
	int file;
	void* addr;
#else
	#error Unknown OS.
#endif

	FileMapping() = default;
};

} // namespace cero
