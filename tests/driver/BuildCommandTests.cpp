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
	const auto system_error = "Access is denied.";
#else
	const auto system_error = "No such device";
#endif

	ExhaustiveReporter r;
	r.set_source_name(".");
	r.expect(0, 0, cero::Message::CouldNotOpenFile, cero::MessageArgs(system_error));

	cero::Configuration config;
	auto source = cero::Source::from_file(".", config);
	cero::build_source(source, config, r);
}

} // namespace tests
