#pragma once

#include "driver/Config.hpp"
#include "driver/SourceLocation.hpp"
#include "util/MappedFile.hpp"

#include <expected>
#include <optional>
#include <string_view>

namespace cero {

class Source {
public:
	static std::expected<Source, std::error_code> from_file(std::string_view path, const Config& config);

	// Exists primarily for testing purposes, so as to not have to read in a file for every test source.
	static Source from_text(std::string_view text, std::string_view path, const Config& config);

	std::string_view get_text() const;
	std::string_view get_path() const;
	SourceLocation	 locate(uint32_t offset) const;

private:
	std::optional<MappedFile> file;
	std::string_view		  text;
	std::string_view		  path;
	uint32_t				  tab_size;

	Source(std::optional<MappedFile> file, std::string_view text, std::string_view path, uint32_t tab_size);
};

} // namespace cero
