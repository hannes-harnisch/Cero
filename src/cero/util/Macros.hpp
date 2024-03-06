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

#include "Fail.hpp"

#ifndef NDEBUG
	#define CERO_ASSERT_DEBUG(condition, info)                                                                                 \
		do {                                                                                                                   \
			if (!(condition)) {                                                                                                \
				CERO_DEBUG_BREAK();                                                                                            \
				fail_assert(info);                                                                                             \
			}                                                                                                                  \
		} while (false)
#else
	#define CERO_ASSERT_DEBUG(condition, ...) static_cast<void>(condition)
#endif
