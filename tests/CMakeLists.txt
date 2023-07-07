set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE TESTS_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*")

add_executable(Tests ${TESTS_SRC})

target_link_libraries(Tests PRIVATE Cero)

target_include_directories(Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(Tests PRIVATE ${CMAKE_SOURCE_DIR}/external/doctest)

set_target_properties(Tests PROPERTIES
        PRECOMPILE_HEADERS ${CMAKE_SOURCE_DIR}/src/PrecompiledHeader.hpp)
