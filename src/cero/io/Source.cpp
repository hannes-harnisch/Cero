#include "Source.hpp"

namespace cero {

std::string_view SourceGuard::get_text() const {
	return source_code_;
}

size_t SourceGuard::get_length() const {
	return source_code_.length();
}

std::string_view SourceGuard::get_name() const {
	return name_;
}

CodeLocation SourceGuard::locate(SourceOffset offset) const {
	auto range = source_code_.substr(0, offset);

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
	return {name_, line, column};
}

SourceGuard::SourceGuard(std::string_view text, std::string_view source_code, uint8_t tab_size) :
	mapping_(std::nullopt),
	source_code_(text),
	name_(source_code),
	tab_size_(tab_size) {
}

SourceGuard::SourceGuard(FileMapping&& mapping, std::string_view source_code, uint8_t tab_size) :
	mapping_(std::move(mapping)),
	source_code_(mapping_->get_text()),
	name_(source_code),
	tab_size_(tab_size) {
}

Source Source::from_file(std::string_view path, const Configuration& config) {
	return Source(path, {}, config.tab_size);
}

Source Source::from_string(std::string_view name, std::string_view source_code, const Configuration& config) {
	return Source(name, source_code, config.tab_size);
}

Result<SourceGuard, std::error_condition> Source::lock() const {
	if (source_code_.data() == nullptr) {
		return FileMapping::from(name_).map([&](FileMapping&& file_mapping) -> SourceGuard {
			return SourceGuard(std::move(file_mapping), name_, tab_size_);
		});
	} else {
		return SourceGuard(source_code_, name_, tab_size_);
	}
}

std::string_view Source::get_name() const {
	return name_;
}

Source::Source(std::string_view name, std::string_view source_code, uint8_t tab_size) :
	name_(name),
	source_code_(source_code),
	tab_size_(tab_size) {
}

} // namespace cero
