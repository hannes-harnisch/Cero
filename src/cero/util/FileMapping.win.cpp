#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/SystemError.hpp"
#include "cero/util/WinApi.win.hpp"
#include "cero/util/WinUtil.win.hpp"

namespace cero {

struct FileMappingImpl {
	HANDLE file = INVALID_HANDLE_VALUE;
	size_t size = 0;
	HANDLE mapping = INVALID_HANDLE_VALUE;
	LPVOID addr = nullptr;

	void destroy() const {
		if (addr != nullptr) {
			BOOL success = ::UnmapViewOfFile(addr);
			check(success, fmt::format("could not unmap file, system error: {}", get_last_system_error().message()));
		}
		if (mapping != INVALID_HANDLE_VALUE) {
			BOOL success = ::CloseHandle(mapping);
			check(success,
				  fmt::format("could not close file mapping handle, system error: {}", get_last_system_error().message()));
		}
		if (file != INVALID_HANDLE_VALUE) {
			BOOL success = ::CloseHandle(file);
			check(success, fmt::format("could not close file handle, system error: {}", get_last_system_error().message()));
		}
	}
};

Result<FileMapping, std::error_condition> FileMapping::from(std::string_view path) {
	auto path_utf16 = windows::utf8_to_utf16(path);

	FileMapping f;
	f.impl_->file = ::CreateFileW(path_utf16.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL, nullptr);
	if (f.impl_->file == INVALID_HANDLE_VALUE) {
		return get_last_system_error();
	}

	LARGE_INTEGER file_size;
	BOOL success = ::GetFileSizeEx(f.impl_->file, &file_size);
	if (!success) {
		return get_last_system_error();
	}
	f.impl_->size = static_cast<size_t>(file_size.QuadPart);

	if (f.impl_->size > 0) {
		HANDLE mapping = ::CreateFileMappingW(f.impl_->file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (mapping == nullptr) {
			return get_last_system_error();
		}
		f.impl_->mapping = mapping;

		f.impl_->addr = ::MapViewOfFile(f.impl_->mapping, FILE_MAP_READ, 0, 0, 0);
		if (f.impl_->addr == nullptr) {
			return get_last_system_error();
		}
	}

	return f;
}

std::string_view FileMapping::get_text() const {
	if (impl_->size == 0) {
		return "";
	} else {
		return std::string_view(static_cast<const char*>(impl_->addr), impl_->size);
	}
}

size_t FileMapping::get_size() const {
	return impl_->size;
}

FileMapping::FileMapping() = default;
FileMapping::~FileMapping() = default;
FileMapping::FileMapping(FileMapping&&) noexcept = default;
FileMapping& FileMapping::operator=(FileMapping&&) noexcept = default;

} // namespace cero
