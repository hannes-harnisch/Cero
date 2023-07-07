#include "MappedFile.hpp"

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
		if (::close(file_descriptor) == -1)
			fail_result(std::format("Could not close file (system error {}).", errno));
	}

} // namespace

std::expected<MappedFile, std::error_code> MappedFile::from(std::string_view path) {
	int file_descriptor = ::open(path.data(), O_RDONLY);
	if (file_descriptor == -1)
		return unexpected_error();

	struct stat file_stats = {};
	if (::fstat(file_descriptor, &file_stats) == -1) {
		close_file(file_descriptor);
		return unexpected_error();
	}

	void*  addr = nullptr;
	size_t size = static_cast<size_t>(file_stats.st_size);
	if (size != 0) {
		addr = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
		if (addr == MAP_FAILED) {
			close_file(file_descriptor);
			return unexpected_error();
		}
	}

	void* file = reinterpret_cast<void*>(static_cast<uintptr_t>(file_descriptor));
	return MappedFile(file, nullptr, addr, size);
}

MappedFile::~MappedFile() {
	if (addr != nullptr) {
		if (::munmap(addr, size) == -1)
			fail_result(std::format("Could not unmap file (system error {}).", errno));
	}

	int descriptor = static_cast<int>(reinterpret_cast<uintptr_t>(file));
	close_file(descriptor);
}

MappedFile::MappedFile(MappedFile&& other) noexcept :
	file(other.file),
	mapping(other.mapping),
	addr(other.addr),
	size(other.size) {
	other.file	  = reinterpret_cast<void*>(uintptr_t(-1));
	other.mapping = nullptr;
	other.addr	  = nullptr;
}

} // namespace cero
