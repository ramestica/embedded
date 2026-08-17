# application/CMakeLists.txt
#
# Modules ported from the legacy recursive-make tree (see application/Makefile
# for the modules still unconverted). Requires configuring with the
# conan-cortex-m4-<build_type> preset (see CMakeUserPresets.json), which
# supplies the arm-none-eabi cross toolchain via Conan's cortex-m4 profile.

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Board selection is hardcoded to tivac-minim4 (TM4C123GH6PM) for now — the
# only board actually in use. Ported from application/Kit/include/Makefile.mk;
# revisit once a second board needs to build from this tree.
add_compile_definitions(
    "TARGET_tivac-minim4"
    F_CPU=80000000UL
    F_CPU_CYCLES_PER_US=80UL
)

#
# Support
#
add_subdirectory(Support/ErrorCodes)

#___oOo___
