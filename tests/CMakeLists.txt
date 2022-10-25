set(CMAKE_CXX_STANDARD		    23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS		OFF)

file(GLOB_RECURSE TEST_SRC *.cpp *.hpp)

add_executable(Tests ${TEST_SRC})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/cero)
include_directories(${CMAKE_SOURCE_DIR}/extern/doctest)
target_link_libraries(Tests Cero)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_SRC})