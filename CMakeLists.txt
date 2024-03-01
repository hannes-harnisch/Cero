cmake_minimum_required(VERSION 3.21)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not recommended. Consider re-running CMake with `-B build` instead.")
endif ()

project(Cero)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)

include(FetchContent)

message("Fetching dependency: fmtlib")
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

message("Fetching dependency: doctest")
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

if (WIN32)
    add_compile_definitions(CERO_WINDOWS)
elseif (UNIX)
    add_compile_definitions(CERO_UNIX)
endif ()

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow=local -Wconversion -Wsign-conversion)
endif ()

add_subdirectory(src)
add_subdirectory(tests)
