cmake_minimum_required(VERSION 3.21)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not recommended. Consider re-running CMake with `-B build` instead.")
endif ()

project(Cero)

set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if (WIN32)
    add_compile_definitions(CERO_WINDOWS)
elseif (UNIX)
    add_compile_definitions(CERO_UNIX)
endif ()

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

add_subdirectory(src)
add_subdirectory(tests)
