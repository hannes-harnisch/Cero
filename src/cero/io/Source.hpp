#pragma once

#include "cero/io/CodeLocation.hpp"
#include "cero/io/Configuration.hpp"
#include "cero/util/FileMapping.hpp"
#include "cero/util/Result.hpp"

#include <optional>
#include <string_view>

namespace cero {

/// Minimum number of bits required to represent any offset into a Cero source input. It is 24 bits so that an offset value can
/// fit into a 32-bit integer, leaving 8 bits available for metadata serving various purposes.
constexpr inline size_t SourceOffsetBits = 24;

/// Recommended type to use for values and bit fields representing offsets into source code.
using SourceOffset = uint32_t;

/// Maximum allowed byte size of a Cero source file (circa 16 MiB), derived from the number of bits needed to represent the
/// source offset of any valid token including the end-of-file token, whose offset equals the source length.
constexpr inline SourceOffset MaxSourceLength = (1 << SourceOffsetBits) - 1;

/// Allows access to the source code for processing, possibly from a memory-mapped file. Closes the memory-mapped file when
/// going out of scope.
class SourceGuard {
public:
	/// Gets a view of the source code.
	std::string_view get_text() const;

	/// Gets the length of the source input.
	size_t get_length() const;

	/// Gets the name of the original source input.
	std::string_view get_name() const;

	/// Determine the line and column that a given source offset corresponds to.
	CodeLocation locate(SourceOffset offset) const;

private:
	std::optional<FileMapping> mapping_;
	std::string_view source_code_;
	std::string_view name_;
	uint8_t tab_size_;

	SourceGuard(std::string_view source_code, std::string_view name, uint8_t tab_size);
	SourceGuard(FileMapping&& mapping, std::string_view name, uint8_t tab_size);

	friend class Source;
};

/// Represents a source input for the compiler, either originating from a file or from a given string of source code. Accessing
/// the source code requires using the lock method. Certain source-specific metadata is determined by the
/// configuration, such as intended tab size.
class Source {
public:
	/// Creates a source representing a source file at the given path. The path will become the name of the source.
	static Source from_file(std::string_view path, const Configuration& config);

	/// Creates a source directly from a string containing source code. Useful for testing so that there doesn't have to be an
	/// extra source file for a test.
	static Source from_string(std::string_view name, std::string_view source_code, const Configuration& config);

	/// If the source represents a file, tries to open it as a memory-mapped file that will be closed when the guard goes out of
	/// scope. If the operation fails, the system error code is returned. Locking source objects created directly from strings
	/// will never fail.
	Result<SourceGuard, std::error_code> lock() const;

	/// Gets the name of the source input.
	std::string_view get_name() const;

private:
	std::string_view name_;
	std::string_view source_code_;
	uint8_t tab_size_;

	Source(std::string_view name, std::string_view text, uint8_t tab_size);
};

} // namespace cero
