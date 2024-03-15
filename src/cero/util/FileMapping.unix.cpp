#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/SystemError.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cero {

namespace {

	void close_file(int file_descriptor) {
		if (::close(file_descriptor) == -1) {
			fail_result(fmt::format("Could not close file. System error: {}", get_system_error_message()));
		}
	}

} // namespace

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	int file = ::open(path.data(), O_RDONLY);
	if (file == -1) {
		return get_system_error();
	}

	struct stat file_stats = {};
	if (::fstat(file, &file_stats) == -1) {
		close_file(file);
		return get_system_error();
	}

	const void* addr = "";
	const size_t size = static_cast<size_t>(file_stats.st_size);
	if (size > 0) {
		addr = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file, 0);
		if (addr == MAP_FAILED) {
			close_file(file);
			return get_system_error();
		}
	}

	FileMapping fm {};
	fm.size_ = size;
	fm.file_ = file;
	fm.addr_ = addr;
	return fm;
}

FileMapping::~FileMapping() {
	if (addr_ != nullptr && size_ > 0) {
		if (::munmap(const_cast<void*>(addr_), size_) == -1) {
			fail_result(fmt::format("Could not unmap file. System error: {}", get_system_error_message()));
		}
	}

	if (file_ != -1) {
		close_file(file_);
	}
}

FileMapping::FileMapping(FileMapping&& other) noexcept :
	size_(other.size_),
	file_(std::exchange(other.file_, -1)),
	addr_(std::exchange(other.addr_, nullptr)) {
}

} // namespace cero
