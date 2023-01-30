set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE CERO_SRC CONFIGURE_DEPENDS *.cpp *.hpp)
file(GLOB_RECURSE CERO_INCLUDE CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*")
file(GLOB_RECURSE PCH_FILES "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cmake_pch.*")

add_library(Cero ${CERO_SRC})

target_include_directories(Cero PUBLIC ${CMAKE_SOURCE_DIR}/external/magic_enum/include)
target_include_directories(Cero PUBLIC ${CMAKE_SOURCE_DIR}/external/pfr/include)
target_include_directories(Cero PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Cero PUBLIC ${CMAKE_SOURCE_DIR}/src)

source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "src" FILES ${CERO_SRC})
source_group(TREE "${CMAKE_SOURCE_DIR}/include" PREFIX "include" FILES ${CERO_INCLUDE})
source_group("generated" FILES ${PCH_FILES})

if (WIN32)
    add_compile_definitions(CERO_WINDOWS)
endif ()
