# Build freetype for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libfreetype_src

    ../../src/autofit/autofit.c
    ../../src/autohint/autohint.c
    ../../src/base/ftbase.c
    ../../src/base/ftglyph.c
    ../../src/base/ftinit.c
    ../../src/base/ftmm.c
    ../../src/base/ftsystem.c
    ../../src/bdf/bdf.c
    ../../src/cache/ftcache.c
    ../../src/cff/cff.c
    ../../src/cid/type1cid.c
    ../../src/gzip/ftgzip.c
    ../../src/lzw/ftlzw.c
    ../../src/pcf/pcf.c
    ../../src/pfr/pfr.c
    ../../src/psaux/psaux.c
    ../../src/pshinter/pshinter.c
    ../../src/psnames/psmodule.c
    ../../src/raster/raster.c
    ../../src/sfnt/sfnt.c
    ../../src/smooth/smooth.c
    ../../src/truetype/truetype.c
    ../../src/type1/type1.c
    ../../src/type42/type42.c
    ../../src/windows/ftdebug.c
    ../../src/winfonts/winfnt.c
    )

add_library (freetype STATIC ${libfreetype_src})

target_include_directories (freetype PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# Copy the lib into the right spot.
install (TARGETS freetype
    ARCHIVE
    DESTINATION ${CMAKE_BINARY_DIR}
    )
