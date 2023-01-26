#pragma once

namespace cero
{

enum class ExitCode
{
	Success		  = 0,
	Usage		  = 64,
	DataError	  = 65,
	NoInput		  = 66,
	InternalError = 70,
	OsError		  = 71,
	CannotCreate  = 73,
	IoError		  = 74,
};

} // namespace cero