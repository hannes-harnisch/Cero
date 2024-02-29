#pragma once

#include "cero/io/CodeLocation.hpp"
#include "cero/io/Config.hpp"
#include "cero/util/FileMapping.hpp"
#include "cero/util/Result.hpp"

#include <optional>
#include <string_view>

namespace cero {

// Number of bits required to represent any offset into a Cero source file. Chosen such that we can put any source header into a
// 32-bit integer while having 8 bits left over for other metadata.
constexpr inline size_t SourceOffsetBits = 24;

// Recommended type to use for values and bit fields representing offsets into a source file.
using SourceOffset = uint32_t;

// Maximum allowed byte size of a Cero source file (16 MiB), derived from the number of bits needed to represent the source
// offset of any valid token including the end-of-file token, whose offset equals the source length.
constexpr inline SourceOffset MaxSourceLength = (1 << SourceOffsetBits) - 1;

class LockedSource {
public:
	std::string_view get_text() const;

	size_t get_length() const;

	std::string_view get_path() const;

	CodeLocation locate(SourceOffset offset) const;

private:
	std::optional<FileMapping> mapping_;
	std::string_view text_;
	std::string_view path_;
	uint8_t tab_size_;

	explicit LockedSource(std::string_view text, std::string_view path, uint8_t tab_size);
	explicit LockedSource(FileMapping mapping, std::string_view path, uint8_t tab_size);

	friend class Source;
};

class Source {
public:
	static Source from_file(std::string_view path, const Config& config);

	// Exists primarily for testing purposes, to not have to read in a file for every test source.
	static Source from_text(std::string_view path, std::string_view text, const Config& config);

	std::string_view get_path() const;

	Result<LockedSource, std::error_code> lock() const;

private:
	std::string_view path_;
	std::string_view text_;
	uint8_t tab_size_;

	Source(std::string_view path, std::string_view text, uint8_t tab_size);
};

} // namespace cero
