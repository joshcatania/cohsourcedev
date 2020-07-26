# Build liblua for CoX.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    liblua_src

    ../../src/lapi.c
    ../../src/lauxlib.c
    ../../src/lbaselib.c
    ../../src/lcode.c
    ../../src/ldblib.c
    ../../src/ldebug.c
    ../../src/ldo.c
    ../../src/ldump.c
    ../../src/lfunc.c
    ../../src/lgc.c
    ../../src/linit.c
    ../../src/liolib.c
    ../../src/llex.c
    ../../src/lmathlib.c
    ../../src/lmem.c
    ../../src/loadlib.c
    ../../src/lobject.c
    ../../src/lopcodes.c
    ../../src/loslib.c
    ../../src/lparser.c
    ../../src/lstate.c
    ../../src/lstring.c
    ../../src/lstrlib.c
    ../../src/ltable.c
    ../../src/ltablib.c
    ../../src/ltm.c
    ../../src/lua.c
    ../../src/luac.c
    ../../src/lundump.c
    ../../src/lvm.c
    ../../src/lzio.c
    ../../src/print.c
    )

add_library (lua STATIC ${liblua_src})

# Copy the lib into the right spot.
install (TARGETS lua
    ARCHIVE
    DESTINATION ${CMAKE_BINARY_DIR}
    )
