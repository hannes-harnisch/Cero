set(CMAKE_CXX_STANDARD		    23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

file(GLOB_RECURSE TEST_SRC *.cpp *.hpp)
file(GLOB_RECURSE PCH_FILES ${CMAKE_CURRENT_SOURCE_DIR}${CMAKE_FILES_DIRECTORY}/cmake_pch.*)

add_executable(Tests ${TEST_SRC})

target_include_directories(Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Tests PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(Tests PRIVATE ${CMAKE_SOURCE_DIR}/extern/doctest)
target_link_libraries(Tests Cero)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_SRC})
source_group("generated" FILES ${PCH_FILES})