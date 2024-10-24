#include "FileMapping.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/SystemError.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cero {

void FileMapping::close_fd(int fd) {
	int result = ::close(fd);
	if (result == -1) {
		fail_result(fmt::format("Could not close file. System error: {}", get_system_error_message()));
	}
}

int FileMapping::null_fd() {
	return -1;
}

void FileMapping::unmap(Mapping map) {
	int result = ::munmap(map.addr, map.size);
	if (result == -1) {
		fail_result(fmt::format("Could not unmap file. System error: {}", get_system_error_message()));
	}
}

FileMapping::Mapping FileMapping::null_mapping() {
	return {MAP_FAILED, 0};
}

Result<FileMapping, std::error_code> FileMapping::from(std::string_view path) {
	std::string path_nt(path);
	UniqueFd file(::open(path_nt.c_str(), O_RDONLY));
	if (file.get() == -1) {
		return get_system_error();
	}

	struct stat file_stats = {};
	int result = ::fstat(file.get(), &file_stats);
	if (result == -1) {
		return get_system_error();
	}
	const size_t size = static_cast<size_t>(file_stats.st_size);

	UniqueMapping mapping;
	if (size > 0) {
		void* addr = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file.get(), 0);
		if (addr == MAP_FAILED) {
			return get_system_error();
		}
		mapping.reset({addr, size});
	}

	FileMapping fm;
	fm.file_ = std::move(file);
	fm.mapping_ = std::move(mapping);
	return fm;
}

std::string_view FileMapping::get_text() const {
	if (mapping_->size == 0) {
		return "";
	} else {
		return std::string_view(static_cast<const char*>(mapping_->addr), mapping_->size);
	}
}

size_t FileMapping::get_size() const {
	return mapping_->size;
}

} // namespace cero
