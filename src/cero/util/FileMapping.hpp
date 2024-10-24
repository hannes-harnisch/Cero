#pragma once

#include "cero/util/Result.hpp"
#include "cero/util/Unique.hpp"

#include <string_view>
#include <system_error>

namespace cero {

class FileMapping {
public:
	static Result<FileMapping, std::error_code> from(std::string_view path);

	std::string_view get_text() const;
	size_t get_size() const;

private:
#if CERO_WINDOWS

	static void close_handle(void*);
	static void* null_handle();
	using UniqueHandle = Unique<void*, close_handle, null_handle>;

	static void unmap(void*);
	static void* null_addr();
	using UniqueMapAddr = Unique<void*, unmap, null_addr>;

	UniqueHandle file_;
	UniqueHandle mapping_;
	UniqueMapAddr addr_;
	size_t size_;

#elif CERO_UNIX

	static void close_fd(int);
	static int null_fd();
	using UniqueFd = Unique<int, close_fd, null_fd>;

	struct Mapping {
		void* addr;
		size_t size;

		bool operator==(const Mapping&) const = default;
	};

	static void unmap(Mapping);
	static Mapping null_mapping();
	using UniqueMapping = Unique<Mapping, unmap, null_mapping>;

	UniqueFd file_;
	UniqueMapping mapping_;

#endif
};

} // namespace cero
