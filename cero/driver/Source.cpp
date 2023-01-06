#include "Source.hpp"

namespace cero
{

namespace
{
	std::optional<std::string> get_file_content(std::string_view path_text)
	{
		std::ifstream file(std::string(path_text), std::ios::in | std::ios::binary);
		if (file.fail())
			return {};

		std::string content;
		file.seekg(0, std::ios::end);

		auto overwrite = [&](char* buffer, size_t size)
		{
			file.seekg(0, std::ios::beg);
			file.read(buffer, size);
			return size;
		};
		content.resize_and_overwrite(file.tellg(), overwrite);
		return content;
	}
} // namespace

std::optional<Source> Source::from_file(std::string_view path, const Config& config)
{
	auto source_text = get_file_content(path);

	if (source_text.has_value())
		return Source(std::move(*source_text), path, config);

	return {};
}

Source::Source(std::string source_text, std::string_view path, const Config& config) :
	text(std::move(source_text)),
	path(path),
	tab_size(config.tab_size)
{}

SourceLocation Source::locate(Source::Iterator cursor) const
{
	std::string_view range(text.begin(), cursor);

	uint32_t line	= 1;
	uint32_t column = 1;
	for (char c : range)
	{
		if (c == '\t')
			column += tab_size;
		else if (c == '\n')
		{
			++line;
			column = 1;
		}
		else
			++column;
	}

	return {line, column, path};
}

Source::Iterator Source::begin() const
{
	return text.begin();
}

Source::Iterator Source::end() const
{
	return text.end();
}

std::string_view Source::get_text() const
{
	return text;
}

std::string_view Source::get_path() const
{
	return path;
}

} // namespace cero