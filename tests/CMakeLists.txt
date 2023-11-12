file(GLOB_RECURSE CERO_TESTS_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*")

add_executable(CeroTests ${CERO_TESTS_SRC})

target_link_libraries(CeroTests PRIVATE Cero)

target_include_directories(CeroTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CeroTests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(CeroTests PRIVATE ${CMAKE_SOURCE_DIR}/external/doctest)

set_target_properties(CeroTests PROPERTIES
        PRECOMPILE_HEADERS ${CMAKE_SOURCE_DIR}/src/PrecompiledHeader.hpp)
