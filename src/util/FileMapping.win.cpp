#include "FileMapping.hpp"

#include "util/Fail.hpp"
#include "util/System.win.hpp"
#include "util/SystemUtil.win.hpp"

namespace cero {

namespace {

	std::unexpected<std::error_code> unexpected_error() {
		int error = static_cast<int>(::GetLastError());
		return std::unexpected(std::error_code(error, std::system_category()));
	}

	void close_file(HANDLE file) {
		if (!::CloseHandle(file)) {
			fail_result(std::format("Could not close file (system error {}).", ::GetLastError()));
		}
	}

	void close_mapping(HANDLE map_handle) {
		if (!::CloseHandle(map_handle)) {
			fail_result(std::format("Could not close file mapping (system error {}).", ::GetLastError()));
		}
	}

} // namespace

std::expected<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	auto w_path = windows::utf8_to_utf16(path);
	auto file = ::CreateFileW(w_path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
							  nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return unexpected_error();
	}

	LARGE_INTEGER file_size;
	if (!::GetFileSizeEx(file, &file_size)) {
		close_file(file);
		return unexpected_error();
	}

	HANDLE map_handle = INVALID_HANDLE_VALUE;
	LPVOID addr = nullptr;
	size_t size = static_cast<size_t>(file_size.QuadPart);
	if (size != 0) {
		map_handle = ::CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (map_handle == nullptr) {
			close_file(file);
			return unexpected_error();
		}

		addr = ::MapViewOfFile(map_handle, FILE_MAP_READ, 0, 0, 0);
		if (addr == nullptr) {
			close_mapping(map_handle);
			close_file(file);
			return unexpected_error();
		}
	}

	FileMapping f_map;
	f_map.size = size;
	f_map.file = file;
	f_map.map_handle = map_handle;
	f_map.map_addr = addr;
	return f_map;
}

FileMapping::~FileMapping() {
	if (map_addr != nullptr) {
		if (!::UnmapViewOfFile(map_addr)) {
			fail_result(std::format("Could not unmap file (system error {}).", ::GetLastError()));
		}
	}

	close_mapping(map_handle);
	close_file(file);
}

FileMapping::FileMapping(FileMapping&& other) noexcept :
	size(other.size),
	file(other.file),
	map_handle(other.map_handle),
	map_addr(other.map_addr) {
	other.file = INVALID_HANDLE_VALUE;
	other.map_handle = INVALID_HANDLE_VALUE;
	other.map_addr = nullptr;
}

} // namespace cero
