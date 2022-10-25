#pragma once

#include <driver/Driver.hpp>
#include <driver/Reporter.hpp>
#include <driver/Source.hpp>
#include <util/Traits.hpp>

#include <doctest/doctest.h>

class ExhaustiveReporter : public Reporter,
						   public Immovable
{
public:
	static ExhaustiveReporter from(std::string_view file_path)
	{
		return ExhaustiveReporter(Source::from(file_path).value());
	}

	ExhaustiveReporter(std::string source) :
		ExhaustiveReporter(Source(std::move(source)))
	{}

	~ExhaustiveReporter()
	{
		CHECK(!has_reports());
	}

private:
	ExhaustiveReporter(const Source& source) :
		Reporter(build_file(source))
	{}
};
