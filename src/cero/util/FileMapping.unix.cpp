#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cero {

namespace {

	std::error_code get_error_code() {
		return std::error_code(errno, std::system_category());
	}

	void close_file(int file_descriptor) {
		if (::close(file_descriptor) == -1) {
			fail_result(fmt::format("Could not close file (system error {}).", errno));
		}
	}

} // namespace

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	int file = ::open(path.data(), O_RDONLY);
	if (file == -1) {
		return get_error_code();
	}

	struct stat file_stats = {};
	if (::fstat(file, &file_stats) == -1) {
		close_file(file);
		return get_error_code();
	}

	const void* addr = "";
	const size_t size = static_cast<size_t>(file_stats.st_size);
	if (size > 0) {
		addr = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file, 0);
		if (addr == MAP_FAILED) {
			close_file(file);
			return get_error_code();
		}
	}

	FileMapping f_map;
	f_map.size_ = size;
	f_map.file_ = file;
	f_map.addr_ = addr;
	return f_map;
}

FileMapping::~FileMapping() {
	if (addr_ != nullptr && size_ > 0) {
		if (::munmap(const_cast<void*>(addr_), size_) == -1) {
			fail_result(fmt::format("Could not unmap file (system error {}).", errno));
		}
	}

	close_file(file_);
}

FileMapping::FileMapping(FileMapping&& other) noexcept :
	size_(other.size_),
	file_(other.file_),
	addr_(other.addr_) {
	other.file_ = -1;
	other.addr_ = nullptr;
}

} // namespace cero
