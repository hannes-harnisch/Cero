#include "Source.hpp"

namespace cero {

std::string_view LockedSource::get_text() const {
	return text_;
}

size_t LockedSource::get_length() const {
	return text_.length();
}

std::string_view LockedSource::get_path() const {
	return path_;
}

CodeLocation LockedSource::locate(SourceOffset offset) const {
	auto range = text_.substr(0, offset);

	const auto line = static_cast<uint32_t>(std::count(range.begin(), range.end(), '\n') + 1);

	const size_t line_index = range.find_last_of('\n');
	if (line_index != std::string_view::npos) {
		range = range.substr(line_index + 1);
	}

	uint32_t column = 1;
	for (char c : range) {
		if (c == '\t') {
			column += tab_size_;
		} else {
			++column;
		}
	}
	return {path_, line, column};
}

LockedSource::LockedSource(std::string_view text, std::string_view path, uint8_t tab_size) :
	mapping_(std::nullopt),
	text_(text),
	path_(path),
	tab_size_(tab_size) {
}

LockedSource::LockedSource(FileMapping mapping, std::string_view path, uint8_t tab_size) :
	mapping_(std::move(mapping)),
	text_(mapping_->get_text()),
	path_(path),
	tab_size_(tab_size) {
}

Source Source::from_file(std::string_view path, const Config& config) {
	return Source(path, {}, config.tab_size);
}

Source Source::from_text(std::string_view path, std::string_view text, const Config& config) {
	return Source(path, text, config.tab_size);
}

std::string_view Source::get_path() const {
	return path_;
}

Result<LockedSource, std::error_code> Source::lock() const {
	if (text_.data() == nullptr) {
		return FileMapping::from(path_).map([&](FileMapping&& file_mapping) -> LockedSource {
			return LockedSource(std::move(file_mapping), path_, tab_size_);
		});
	} else {
		return LockedSource(text_, path_, tab_size_);
	}
}

Source::Source(std::string_view path, std::string_view text, uint8_t tab_size) :
	path_(path),
	text_(text),
	tab_size_(tab_size) {
}

} // namespace cero
