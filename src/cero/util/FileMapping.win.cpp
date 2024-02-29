#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/WinApi.win.hpp"
#include "cero/util/WinUtil.win.hpp"

namespace cero {

namespace {

	std::error_code last_error() {
		int error = static_cast<int>(::GetLastError());
		return std::error_code(error, std::system_category());
	}

	void close_file(HANDLE file) {
		if (!::CloseHandle(file)) {
			fail_result(fmt::format("Could not close file (system error {}).", ::GetLastError()));
		}
	}

	void close_mapping(HANDLE mapping) {
		if (!::CloseHandle(mapping)) {
			fail_result(fmt::format("Could not close file mapping (system error {}).", ::GetLastError()));
		}
	}

} // namespace

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	auto w_path = windows::utf8_to_utf16(path);
	auto file = ::CreateFileW(w_path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
							  nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return last_error();
	}

	LARGE_INTEGER file_size;
	if (!::GetFileSizeEx(file, &file_size)) {
		close_file(file);
		return last_error();
	}

	HANDLE mapping = INVALID_HANDLE_VALUE;
	const void* addr = "";
	const size_t size = static_cast<size_t>(file_size.QuadPart);
	if (size != 0) {
		mapping = ::CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (mapping == nullptr) {
			close_file(file);
			return last_error();
		}

		addr = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
		if (addr == nullptr) {
			close_mapping(mapping);
			close_file(file);
			return last_error();
		}
	}

	FileMapping f_map;
	f_map.size_ = size;
	f_map.file_ = file;
	f_map.mapping_ = mapping;
	f_map.addr_ = addr;
	return f_map;
}

FileMapping::~FileMapping() {
	if (addr_ != nullptr) {
		if (!::UnmapViewOfFile(addr_)) {
			fail_result(fmt::format("Could not unmap file (system error {}).", ::GetLastError()));
		}
	}

	close_mapping(mapping_);
	close_file(file_);
}

FileMapping::FileMapping(FileMapping&& other) noexcept :
	size_(other.size_),
	file_(other.file_),
	mapping_(other.mapping_),
	addr_(other.addr_) {
	other.file_ = INVALID_HANDLE_VALUE;
	other.mapping_ = INVALID_HANDLE_VALUE;
	other.addr_ = nullptr;
}

} // namespace cero
