#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/SystemError.hpp"
#include "cero/util/WinApi.win.hpp"
#include "cero/util/WinUtil.win.hpp"

namespace cero {

static void close_file(HANDLE file) {
	if (!::CloseHandle(file)) {
		fail_result(fmt::format("Could not close file. System error: {}", get_system_error_message()));
	}
}

static void close_mapping(HANDLE mapping) {
	if (!::CloseHandle(mapping)) {
		fail_result(fmt::format("Could not close file mapping. System error: {}", get_system_error_message()));
	}
}

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	auto w_path = windows::utf8_to_utf16(path);
	auto file = ::CreateFileW(w_path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
							  nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return get_system_error();
	}

	LARGE_INTEGER file_size;
	if (!::GetFileSizeEx(file, &file_size)) {
		close_file(file);
		return get_system_error();
	}

	HANDLE mapping = INVALID_HANDLE_VALUE;
	const void* addr = "";
	const size_t size = static_cast<size_t>(file_size.QuadPart);
	if (size > 0) {
		mapping = ::CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (mapping == nullptr) {
			close_file(file);
			return get_system_error();
		}

		addr = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
		if (addr == nullptr) {
			close_mapping(mapping);
			close_file(file);
			return get_system_error();
		}
	}

	FileMapping fm {};
	fm.size_ = size;
	fm.file_ = file;
	fm.mapping_ = mapping;
	fm.addr_ = addr;
	return fm;
}

FileMapping::~FileMapping() {
	if (addr_ != nullptr && size_ > 0) {
		if (!::UnmapViewOfFile(addr_)) {
			fail_result(fmt::format("Could not unmap file. System error: {}", get_system_error_message()));
		}
	}

	if (mapping_ != INVALID_HANDLE_VALUE) {
		close_mapping(mapping_);
	}
	if (file_ != INVALID_HANDLE_VALUE) {
		close_file(file_);
	}
}

FileMapping::FileMapping(FileMapping&& other) noexcept :
	size_(other.size_),
	file_(std::exchange(other.file_, INVALID_HANDLE_VALUE)),
	mapping_(std::exchange(other.mapping_, INVALID_HANDLE_VALUE)),
	addr_(std::exchange(other.addr_, nullptr)) {
}

} // namespace cero
