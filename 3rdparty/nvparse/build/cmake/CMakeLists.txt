# Build nvparse for CoX.
#
# This one depends on glew.

cmake_minimum_required (VERSION 3.7)
cmake_policy (SET CMP0054 NEW)

project (cox_portable)

# TODO: Figure out what minimum x64 CPU we actually need.
add_compile_options ("-march=native")
add_compile_options ("-msha")

# Source files
set (
    libnvparse_src

    ../../src/nvparse.cpp
    ../../src/nvparse_errors.cpp
    ../../src/ps1.0_program.cpp
    ../../src/rc1.0_combiners.cpp
    ../../src/rc1.0_final.cpp
    ../../src/rc1.0_general.cpp
    ../../src/ts1.0_inst.cpp
    ../../src/ts1.0_inst_list.cpp
    ../../src/vcp1.0_impl.cpp
    ../../src/vp1.0_impl.cpp
    ../../src/vs1.0_inst.cpp
    ../../src/vs1.0_inst_list.cpp
    ../../src/vsp1.0_impl.cpp
    ../../src/_ps1.0_lexer.cpp
    ../../src/_ps1.0_parser.cpp
    ../../src/_rc1.0_lexer.cpp
    ../../src/_rc1.0_parser.cpp
    ../../src/_ts1.0_lexer.cpp
    ../../src/_ts1.0_parser.cpp
    ../../src/_vs1.0_lexer.cpp
    ../../src/_vs1.0_parser.cpp
    )

add_library (nvparse STATIC ${libnvparse_src})

target_compile_options (nvparse PUBLIC "-Wno-write-strings")
target_include_directories (nvparse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../glew-2.1.0/include)

# Copy the lib into 3rdparty/nvparse/build/cmake.
install (TARGETS nvparse
    ARCHIVE
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
    )
