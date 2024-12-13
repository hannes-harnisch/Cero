#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/SystemError.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cero {

struct FileMappingImpl {
	int fd = -1;
	size_t size = 0;
	void* addr = MAP_FAILED;

	void destroy() const {
		if (fd != -1) {
			int result = ::close(fd);
			if (result == -1) {
				fail_result(fmt::format("Could not close file. System error: {}", get_system_error_message()));
			}
		}

		if (addr != MAP_FAILED) {
			int result = ::munmap(addr, size);
			if (result == -1) {
				fail_result(fmt::format("Could not unmap file. System error: {}", get_system_error_message()));
			}
		}
	}
};

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	std::string path_nt(path);

	FileMapping f;
	f.impl_->fd = ::open(path_nt.c_str(), O_RDONLY);
	if (f.impl_->fd == -1) {
		return get_system_error();
	}

	struct stat file_stats = {};
	int result = ::fstat(f.impl_->fd, &file_stats);
	if (result == -1) {
		return get_system_error();
	}
	f.impl_->size = static_cast<size_t>(file_stats.st_size);

	if (f.impl_->size > 0) {
		f.impl_->addr = ::mmap(nullptr, f.impl_->size, PROT_READ, MAP_PRIVATE, f.impl_->fd, 0);
		if (f.impl_->addr == MAP_FAILED) {
			return get_system_error();
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
