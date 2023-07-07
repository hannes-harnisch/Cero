#include "Source.hpp"

namespace cero {

std::expected<Source, std::error_code> Source::from_file(std::string_view path, const Config& config) {
	auto file = MappedFile::from(path);
	if (file.has_value()) {
		auto text = file->get_text();
		return Source(std::move(*file), text, path, config.tab_size);
	}
	return std::unexpected(file.error());
}

Source Source::from_text(std::string_view text, std::string_view path, const Config& config) {
	return Source(std::nullopt, text, path, config.tab_size);
}

std::string_view Source::get_text() const {
	return text;
}

std::string_view Source::get_path() const {
	return path;
}

SourceLocation Source::locate(uint32_t offset) const {
	std::string_view range = text.substr(0, offset);

	uint32_t line	= 1;
	uint32_t column = 1;
	for (char c : range) {
		if (c == '\t') {
			column += tab_size;
		} else if (c == '\n') {
			++line;
			column = 1;
		} else {
			++column;
		}
	}

	return {path, line, column};
}

Source::Source(std::optional<MappedFile> file, std::string_view text, std::string_view path, uint32_t tab_size) :
	file(std::move(file)),
	text(text),
	path(path),
	tab_size(tab_size) {
}

} // namespace cero
