#include "ExhaustiveReporter.hpp"

#include <cero/driver/Driver.hpp>
#include <doctest/doctest.h>

ExhaustiveReporter ExhaustiveReporter::from(std::string_view file_path)
{
	return ExhaustiveReporter(Source::from(file_path).value());
}

ExhaustiveReporter::ExhaustiveReporter(std::string source_text) :
	ExhaustiveReporter(Source(std::move(source_text)))
{}

ExhaustiveReporter::~ExhaustiveReporter()
{
	CHECK(!has_reports());
}

ExhaustiveReporter::ExhaustiveReporter(const Source& source) :
	Reporter(build_file(source))
{}
