#pragma once

#if __clang__
	#define CERO_COMPILER_CLANG __clang__
#elif _MSC_VER
	#define CERO_COMPILER_MSVC _MSC_VER
#elif __GNUG__
	#define CERO_COMPILER_GCC __GNUC__
#endif

#if CERO_COMPILER_CLANG
	#define CERO_DEBUG_BREAK() __builtin_debugtrap()
#elif CERO_COMPILER_MSVC
	#define CERO_DEBUG_BREAK() __debugbreak()
#elif CERO_COMPILER_GCC
	#include <csignal>
	#define CERO_DEBUG_BREAK() std::raise(SIGTRAP)
#endif

#define CERO_CONCAT_IMPL(A, B) A##B
#define CERO_CONCAT(A, B)	   CERO_CONCAT_IMPL(A, B)

#define CERO_STRINGIFY_IMPL(A) #A
#define CERO_STRINGIFY(A)	   CERO_STRINGIFY_IMPL(A)
