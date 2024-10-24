#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/SystemError.hpp"
#include "cero/util/WinApi.win.hpp"
#include "cero/util/WinUtil.win.hpp"

namespace cero {

void FileMapping::close_handle(HANDLE handle) {
	BOOL success = ::CloseHandle(handle);
	if (!success) {
		fail_result(fmt::format("Could not close handle. System error: {}", get_system_error_message()));
	}
}

HANDLE FileMapping::null_handle() {
	return INVALID_HANDLE_VALUE;
}

void FileMapping::unmap(LPVOID addr) {
	BOOL success = ::UnmapViewOfFile(addr);
	if (!success) {
		fail_result(fmt::format("Could not unmap file. System error: {}", get_system_error_message()));
	}
}

LPVOID FileMapping::null_addr() {
	return nullptr;
}

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	auto path_utf16 = windows::utf8_to_utf16(path);

	UniqueHandle file(::CreateFileW(path_utf16.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, nullptr));
	if (file.get() == INVALID_HANDLE_VALUE) {
		return get_system_error();
	}

	LARGE_INTEGER file_size;
	BOOL success = ::GetFileSizeEx(file.get(), &file_size);
	if (!success) {
		return get_system_error();
	}
	const size_t size = static_cast<size_t>(file_size.QuadPart);

	UniqueHandle mapping;
	UniqueMapAddr addr;
	if (size > 0) {
		HANDLE map_handle = ::CreateFileMappingW(file.get(), nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (map_handle == nullptr) {
			return get_system_error();
		}
		mapping.reset(map_handle);

		addr.reset(::MapViewOfFile(mapping.get(), FILE_MAP_READ, 0, 0, 0));
		if (addr.get() == nullptr) {
			return get_system_error();
		}
	}

	FileMapping fm;
	fm.file_ = std::move(file);
	fm.mapping_ = std::move(mapping);
	fm.addr_ = std::move(addr);
	fm.size_ = size;
	return fm;
}

std::string_view FileMapping::get_text() const {
	if (size_ == 0) {
		return "";
	} else {
		return std::string_view(static_cast<const char*>(addr_.get()), size_);
	}
}

size_t FileMapping::get_size() const {
	return size_;
}

} // namespace cero
