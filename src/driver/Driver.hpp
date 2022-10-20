#pragma once

namespace cero
{
	int run(std::span<std::string_view> args);

	class Driver
	{
	public:
		Driver();
		Driver(std::streambuf* output_stream, std::streambuf* error_stream);

		bool run_command(std::span<std::string_view> args);
	};
}
