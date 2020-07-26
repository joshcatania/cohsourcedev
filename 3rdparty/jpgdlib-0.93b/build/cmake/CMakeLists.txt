# Build jpgd for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libjpgd_src

    ../../src/h2v2.cpp
    ../../src/idct.cpp
    ../../src/jidctfst.cpp
    ../../src/jpegdecoder.cpp
    )

add_library (jpgd STATIC ${libjpgd_src})
target_compile_options (jpgd PUBLIC "-DJPGD_DISABLE_ASM=1")
target_compile_options (jpgd PUBLIC "-DJPGD_USE_STDINT_H=1")
target_compile_options (jpgd PUBLIC "-D_M_X64=1")  # WARNING: Assumes 64-bit build, 32-bit build is toast.

# Copy the lib into 3rdparty/jpgdlib-0.93b/build/cmake.
install (TARGETS jpgd
    ARCHIVE
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    )
