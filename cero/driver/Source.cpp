#include "Source.hpp"

static std::optional<std::string> get_file_content(std::string_view path_text)
{
	std::ifstream in(std::string(path_text), std::ios::in | std::ios::binary);
	if (in.fail())
		return {};

	std::string content;
	in.seekg(0, std::ios::end);
	content.resize_and_overwrite(in.tellg(), [&](char* buffer, size_t size) {
		in.seekg(0, std::ios::beg);
		in.read(buffer, size);
		return size;
	});
	return content;
}

std::optional<Source> Source::from(std::string_view path)
{
	auto source_text = get_file_content(path);

	if (source_text.has_value())
		return Source(std::move(*source_text), path);

	return {};
}

Source::Source(std::string text) :
	text(std::move(text))
{}

SourceLocation Source::locate(Source::Iterator cursor) const
{
	std::string_view range(text.begin(), cursor);

	uint32_t line	= 1;
	uint32_t column = 1;
	for (char c : range)
	{
		++column;
		if (c == '\n')
		{
			++line;
			column = 1;
		}
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

Source::Source(std::string text, std::string_view path) :
	text(std::move(text)),
	path(path)
{}
