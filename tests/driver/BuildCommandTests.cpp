#include "common/ExhaustiveReporter.hpp"
#include "common/Test.hpp"

#include <cero/driver/BuildCommand.hpp>

namespace tests {

CERO_TEST(FileNotFoundForBuildCommand) {
	ExhaustiveReporter r;
	r.set_source_name("FileShouldNotExist.ce");
	r.expect(0, 0, cero::Message::FileNotFound, {});

	cero::Configuration config;
	auto source = cero::Source::from_file("FileShouldNotExist.ce", config);
	cero::build_source(source, config, r);
}

CERO_TEST(CouldNotOpenFileForBuildCommand) {
#if CERO_WINDOWS
	constexpr auto err_code = std::errc::permission_denied;
#else
	constexpr auto err_code = std::errc::no_such_device;
#endif
	const auto err_msg = std::make_error_condition(err_code).message();

	ExhaustiveReporter r;
	r.set_source_name(".");
	r.expect(0, 0, cero::Message::CouldNotOpenFile, cero::MessageArgs(err_msg));

	cero::Configuration config;
	auto source = cero::Source::from_file(".", config);
	cero::build_source(source, config, r);
}

} // namespace tests
