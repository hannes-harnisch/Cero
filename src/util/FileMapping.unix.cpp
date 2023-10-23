#include "FileMapping.hpp"

#include "util/Fail.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cero {

namespace {

	std::unexpected<std::error_code> unexpected_error() {
		return std::unexpected(std::error_code(errno, std::system_category()));
	}

	void close_file(int file_descriptor) {
		if (::close(file_descriptor) == -1) {
			fail_result(std::format("Could not close file (system error {}).", errno));
		}
	}

} // namespace

std::expected<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	int file = ::open(path.data(), O_RDONLY);
	if (file == -1) {
		return unexpected_error();
	}

	struct stat file_stats = {};
	if (::fstat(file, &file_stats) == -1) {
		close_file(file);
		return unexpected_error();
	}

	void* addr = nullptr;
	size_t size = static_cast<size_t>(file_stats.st_size);
	if (size != 0) {
		addr = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
		if (addr == MAP_FAILED) {
			close_file(file);
			return unexpected_error();
		}
	}

	FileMapping f_map;
	f_map.size = size;
	f_map.file = file;
	f_map.map_addr = addr;
	return f_map;
}

FileMapping::~FileMapping() {
	if (map_addr != nullptr) {
		if (::munmap(map_addr, size) == -1) {
			fail_result(std::format("Could not unmap file (system error {}).", errno));
		}
	}

	close_file(file);
}

FileMapping::FileMapping(FileMapping&& other) noexcept :
	size(other.size),
	file(other.file),
	map_addr(other.map_addr) {
	other.file = -1;
	other.map_addr = nullptr;
}

} // namespace cero
