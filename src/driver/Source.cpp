#include "Source.hpp"

namespace cero {

std::string_view SourceLock::get_text() const {
	return text;
}

SourceLock::SourceLock(std::string_view text) :
	mapping(std::nullopt),
	text(text) {
}

SourceLock::SourceLock(FileMapping file_mapping) :
	mapping(std::move(file_mapping)),
	text(mapping->get_text()) {
}

Source Source::from_file(std::string_view path, const Config& config) {
	return Source(path, {}, config.tab_size);
}

Source Source::from_text(std::string_view path, std::string_view text, const Config& config) {
	return Source(path, text, config.tab_size);
}

std::string_view Source::get_path() const {
	return path;
}

std::expected<SourceLock, std::error_code> Source::lock() const {
	if (text.data() == nullptr) {
		return FileMapping::from(path).transform([](FileMapping file_mapping) { return SourceLock(std::move(file_mapping)); });
	} else {
		return SourceLock(text);
	}
}

SourceLocation Source::locate(uint32_t offset) const {
	if (auto guard = lock()) {
		std::string_view range = guard->get_text().substr(0, offset);

		uint32_t line = 1;
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
	// TODO: error handling, rewrite this
	return {};
}

Source::Source(std::string_view path, std::string_view text, uint32_t tab_size) :
	path(path),
	text(text),
	tab_size(tab_size) {
}

} // namespace cero
