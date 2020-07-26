# Build libvorbis for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libvorbis_src

    ../../src/analysis.c
    ../../src/bitrate.c
    ../../src/block.c
    ../../src/codebook.c
    ../../src/envelope.c
    ../../src/floor0.c
    ../../src/floor1.c
    ../../src/info.c
    ../../src/lookup.c
    ../../src/lpc.c
    ../../src/lsp.c
    ../../src/mapping0.c
    ../../src/mdct.c
    ../../src/psy.c
    ../../src/registry.c
    ../../src/res0.c
    ../../src/sharedbook.c
    ../../src/smallft.c
    ../../src/synthesis.c
    ../../src/vorbisfile.c
    ../../src/window.c
    )

add_library (libvorbis STATIC ${libvorbis_src})

target_include_directories(libvorbis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../ogg-1.1.1/include)

# Copy the lib into 3rdparty/vorbis-1.1.0/build/cmake.
install (TARGETS libvorbis
    ARCHIVE
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    )
