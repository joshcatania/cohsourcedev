# Build glew for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libglew_src

    ../../src/glew.c
    )

add_library (glew STATIC ${libglew_src})

target_compile_options (glew PUBLIC "-DGLEW_STATIC=1")
target_include_directories (glew PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# Copy the lib into the right spot.
install (TARGETS glew
    ARCHIVE
    DESTINATION ${CMAKE_BINARY_DIR}
    )
