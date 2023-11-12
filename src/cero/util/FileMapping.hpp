#pragma once

#include "cero/util/Result.hpp"

#include <string_view>
#include <system_error>

namespace cero {

class FileMapping {
public:
	static Result<FileMapping, std::error_code> from(std::string_view path);

	std::string_view get_text() const;
	size_t get_size() const;

	~FileMapping();

	FileMapping(FileMapping&&) noexcept;
	FileMapping& operator=(FileMapping&&) noexcept;

private:
	size_t size_;

#if defined(CERO_WINDOWS)
	void* file_;
	void* mapping_;
	const void* addr_;
#elif defined(CERO_UNIX)
	int file_;
	const void* addr_;
#else
	#error Unknown OS.
#endif

	FileMapping() = default;
};

} // namespace cero
