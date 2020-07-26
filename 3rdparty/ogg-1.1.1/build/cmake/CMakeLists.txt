# Build libogg for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libogg_src

    ../../src/bitwise.c
    ../../src/framing.c
    )

add_library (ogg STATIC ${libogg_src})

target_compile_options (ogg PUBLIC "-DOV_STATICLIB")

# Copy the lib into 3rdparty/ogg-1.1.1/build/cmake.
install (TARGETS ogg
    ARCHIVE
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    )
