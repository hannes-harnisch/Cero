#include "MappedFile.hpp"

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
		if (!::CloseHandle(file))
			fail_result(std::format("Could not close file (system error {}).", ::GetLastError()));
	}

	void close_mapping(HANDLE mapping) {
		if (!::CloseHandle(mapping))
			fail_result(std::format("Could not close file mapping (system error {}).", ::GetLastError()));
	}

} // namespace

std::expected<MappedFile, std::error_code> MappedFile::from(std::string_view path) {
	auto w_path = windows::utf8_to_utf16(path);
	auto file	= ::CreateFileW(w_path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
								nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return unexpected_error();

	LARGE_INTEGER file_size;
	if (!::GetFileSizeEx(file, &file_size)) {
		close_file(file);
		return unexpected_error();
	}

	HANDLE mapping = INVALID_HANDLE_VALUE;
	LPVOID addr	   = nullptr;
	size_t size	   = static_cast<size_t>(file_size.QuadPart);
	if (size != 0) {
		mapping = ::CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (mapping == nullptr) {
			close_file(file);
			return unexpected_error();
		}

		addr = ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
		if (addr == nullptr) {
			close_mapping(mapping);
			close_file(file);
			return unexpected_error();
		}
	}

	return MappedFile(file, mapping, addr, size);
}

MappedFile::~MappedFile() {
	if (addr != nullptr) {
		if (!::UnmapViewOfFile(addr))
			fail_result(std::format("Could not unmap file (system error {}).", ::GetLastError()));
	}

	close_mapping(mapping);
	close_file(file);
}

MappedFile::MappedFile(MappedFile&& other) noexcept :
	file(other.file),
	mapping(other.mapping),
	addr(other.addr),
	size(other.size) {
	other.file	  = INVALID_HANDLE_VALUE;
	other.mapping = INVALID_HANDLE_VALUE;
	other.addr	  = nullptr;
}

} // namespace cero
