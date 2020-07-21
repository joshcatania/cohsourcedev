# Build libjpeg for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libjpeg_src

    ../../src/jaricom.c
    ../../src/jcapimin.c
    ../../src/jcapistd.c
    ../../src/jcarith.c
    ../../src/jccoefct.c
    ../../src/jccolor.c
    ../../src/jcdctmgr.c
    ../../src/jchuff.c
    ../../src/jcinit.c
    ../../src/jcmainct.c
    ../../src/jcmarker.c
    ../../src/jcmaster.c
    ../../src/jcomapi.c
    ../../src/jcparam.c
    ../../src/jcprepct.c
    ../../src/jcsample.c
    ../../src/jctrans.c
    ../../src/jdapimin.c
    ../../src/jdapistd.c
    ../../src/jdarith.c
    ../../src/jdatadst.c
    ../../src/jdatasrc.c
    ../../src/jdcoefct.c
    ../../src/jdcolor.c
    ../../src/jddctmgr.c
    ../../src/jdhuff.c
    ../../src/jdinput.c
    ../../src/jdmainct.c
    ../../src/jdmarker.c
    ../../src/jdmaster.c
    ../../src/jdmerge.c
    ../../src/jdpostct.c
    ../../src/jdsample.c
    ../../src/jdtrans.c
    ../../src/jerror.c
    ../../src/jfdctflt.c
    ../../src/jfdctfst.c
    ../../src/jfdctint.c
    ../../src/jidctflt.c
    ../../src/jidctfst.c
    ../../src/jidctint.c
    ../../src/jmemmgr.c
    ../../src/jmemnobs.c
    ../../src/jquant1.c
    ../../src/jquant2.c
    ../../src/jutils.c
    ../../src/rdbmp.c
    ../../src/rdcolmap.c
    ../../src/rdgif.c
    ../../src/rdppm.c
    ../../src/rdrle.c
    ../../src/rdswitch.c
    ../../src/rdtarga.c
    ../../src/transupp.c
    ../../src/wrbmp.c
    ../../src/wrgif.c
    ../../src/wrppm.c
    ../../src/wrrle.c
    ../../src/wrtarga.c
    )

add_library (libjpeg STATIC ${libjpeg_src})

# Copy the lib into 3rdparty/jpeg-9c/build/cmake.
install (TARGETS libjpeg
    ARCHIVE
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    )
