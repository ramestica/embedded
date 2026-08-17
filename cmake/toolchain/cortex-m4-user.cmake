# Loaded via the cortex-m4 Conan profile's tools.cmake.cmaketoolchain:user_toolchain
# conf, layered on top of the generated conan_toolchain.cmake.
#
# Bare-metal targets have no libc syscall stubs until a real executable links
# one in (ResetISR, a linker script, etc.), so CMake's default "compile a test
# executable" compiler probe fails even for a working compiler.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
