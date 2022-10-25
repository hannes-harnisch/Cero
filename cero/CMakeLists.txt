set(CMAKE_CXX_STANDARD		    23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

file(GLOB_RECURSE CERO_SRC *.cpp *.hpp)
file(GLOB_RECURSE PCH_FILES ${CMAKE_CURRENT_SOURCE_DIR}${CMAKE_FILES_DIRECTORY}/cmake_pch.*)

add_library(Cero ${CERO_SRC})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/extern/magic_enum/include)
set_target_properties(Cero PROPERTIES PRECOMPILE_HEADERS "PCH.hpp")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${CERO_SRC})
source_group("generated" FILES ${PCH_FILES})