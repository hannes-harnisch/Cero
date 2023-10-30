#pragma once

#include "driver/Config.hpp"
#include "driver/SourceLocation.hpp"
#include "util/FileMapping.hpp"

#include <expected>
#include <optional>
#include <string_view>

namespace cero {

class SourceLock {
public:
	std::string_view get_text() const;

private:
	std::optional<FileMapping> mapping;
	std::string_view text;

	explicit SourceLock(std::string_view text);
	explicit SourceLock(FileMapping file_mapping);

	friend class Source;
};

class Source {
public:
	static Source from_file(std::string_view path, const Config& config);

	// Exists primarily for testing purposes, so as to not have to read in a file for every test source.
	static Source from_text(std::string_view path, std::string_view text, const Config& config);

	std::string_view get_path() const;

	std::expected<SourceLock, std::error_code> lock() const;

	SourceLocation locate(uint32_t offset) const;

private:
	std::string_view path;
	std::string_view text;
	uint32_t tab_size;

	Source(std::string_view path, std::string_view text, uint32_t tab_size);
};

} // namespace cero
