# Build libz for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libz_src

    ../../contrib/optimizations/inffast_chunk.c
    ../../src/adler32.c
    ../../src/adler32_simd.c
    ../../src/compress.c
    ../../src/cpu_features.c
    ../../src/crc32.c
    ../../src/crc32_simd.c
    ../../src/crc_folding.c
    ../../src/deflate.c
    ../../src/fill_window_sse.c
    ../../src/gzclose.c
    ../../src/gzlib.c
    ../../src/gzread.c
    ../../src/gzwrite.c
    ../../src/infback.c
    ../../src/inffast.c
    ../../src/inflate.c
    ../../src/inftrees.c
    ../../src/trees.c
    ../../src/uncompr.c
    ../../src/zutil.c
    )

add_library (z STATIC ${libz_src})

target_compile_options (z PUBLIC "-DADLER32_SIMD_SSSE3")
target_compile_options (z PUBLIC "-DINFLATE_CHUNK_SIMD_SSE2")
target_compile_options (z PUBLIC "-DCRC32_SIMD_SSE42_PCLMUL")
target_compile_options (z PUBLIC "-DDEFLATE_FILL_WINDOW_SSE2")
target_include_directories (z PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../src")  # For inffast_chunk.c

# Copy the lib into the right spot.
install (TARGETS z
    ARCHIVE
    DESTINATION ${CMAKE_BINARY_DIR}
    )
