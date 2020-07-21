# Build glew for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

set (GLEW_STATIC 1)

add_compile_options ("-I${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# Source files
set (
    glew_src

    ../../src/glew.c
    )

add_library (glew STATIC ${glew_src})

# Copy the lib into 3rdparty/glew-2.1.0/build/cmake.
install (TARGETS glew
    ARCHIVE
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    )
