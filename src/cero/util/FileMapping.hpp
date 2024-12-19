#pragma once

#include "cero/util/Result.hpp"
#include "cero/util/UniqueImpl.hpp"

#include <string_view>
#include <system_error>

namespace cero {

class FileMapping {
public:
	static Result<FileMapping, std::error_condition> from(std::string_view path);

	std::string_view get_text() const;
	size_t get_size() const;

	~FileMapping();
	FileMapping(FileMapping&&) noexcept;
	FileMapping& operator=(FileMapping&&) noexcept;

private:
	UniqueImpl<struct FileMappingImpl, 32> impl_;

	FileMapping();
};

} // namespace cero
