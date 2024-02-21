file(GLOB_RECURSE CERO_SRC CONFIGURE_DEPENDS "cero/*.cpp")
list(FILTER CERO_SRC EXCLUDE REGEX ".+\\..+\\.cpp")

if (WIN32)
    file(GLOB_RECURSE CERO_WINDOWS_SRC CONFIGURE_DEPENDS "*.win.cpp")
    list(APPEND CERO_SRC ${CERO_WINDOWS_SRC})
elseif (UNIX)
    file(GLOB_RECURSE CERO_UNIX_SRC CONFIGURE_DEPENDS "*.unix.cpp")
    list(APPEND CERO_SRC ${CERO_UNIX_SRC})
endif ()

add_library(Cero ${CERO_SRC})
target_include_directories(Cero PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Cero PUBLIC fmt::fmt)

set_target_properties(Cero PROPERTIES
        PRECOMPILE_HEADERS PrecompiledHeader.hpp)

add_executable(CeroCompiler "Main.cpp")
target_link_libraries(CeroCompiler PRIVATE Cero)

set_target_properties(CeroCompiler PROPERTIES
        PRECOMPILE_HEADERS PrecompiledHeader.hpp)
