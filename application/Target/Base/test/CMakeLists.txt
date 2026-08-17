# Hardware bring-up executable: blinks the status LED and toggles a timer
# ISR. Not part of application/Makefile's MODULES list — historically built
# and flashed by hand, not by the top-level build.
if(NOT EMB_HOST)

emb_module(
    NAME  BaseTest
    TITLE "Target/Base bring-up executable (delayus)"
)

# The linker script picks ROM vs RAM placement via the C preprocessor
# (LOAD_TO_ROM/LOAD_TO_RAM), same as application/Target/Main's legacy
# `install` step. RAM matches Kit/include/Makefile.mk's TARGET_MEMORY
# default.
set(LINKER_SCRIPT_SRC ${CMAKE_SOURCE_DIR}/application/Target/Main/config/tm4c123gh6pm.ld)
set(LINKER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/tm4c123gh6pm.ram.ld)

add_custom_command(
    OUTPUT  ${LINKER_SCRIPT}
    COMMAND ${CMAKE_C_COMPILER} -E -P -x c -DLOAD_TO_RAM ${LINKER_SCRIPT_SRC} -o ${LINKER_SCRIPT}
    DEPENDS ${LINKER_SCRIPT_SRC}
    COMMENT "Preprocessing tm4c123gh6pm.ld for RAM placement"
)
add_custom_target(BaseTest_linker_script DEPENDS ${LINKER_SCRIPT})

emb_executable(
    NAME    delayus
    SOURCES delayus.cpp
)
add_dependencies(delayus BaseTest_linker_script)
set_target_properties(delayus PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# main() (Target/Main) and ResetISR (Target/Base) each pull in the other
# (startup code calls main; main dispatches to Main::setup/loop implemented
# in this executable), and Base needs tivaware's driverlib — genuine
# reference cycles across static libs, not expressible as one-way
# target_link_libraries edges. --start-group/--end-group (passed as if they
# were library names, so they land in the actual library list, not up with
# the other linker flags) makes the linker re-scan until nothing new
# resolves, instead of its default single pass. Every one of our own static
# libs has to be listed explicitly here — anything CMake pulls in on its own
# via transitive PUBLIC deps lands outside the group instead.
target_link_libraries(delayus
    PRIVATE
        -Wl,--start-group
        emb::Main
        emb::Timer
        emb::GPIO
        emb::Base
        emb::Utils
        emb::Interrupts
        emb::SystemViewOff
        emb::ErrorCodes
        ${CMAKE_SOURCE_DIR}/tivaware/driverlib/gcc/libdriver.a
        -Wl,--end-group
)

# No -Wl,--fatal-warnings: nosys.specs' stub _kill/_getpid intentionally warn
# "will always fail" on link, which is expected/benign for a bare-metal
# target, not a real problem.
target_link_options(delayus PRIVATE
    -nostartfiles
    -Wl,--gc-sections
    -Wl,--no-undefined
    -Wl,--undefined=ResetISR
    # newlib syscall stubs (_exit/_kill/_getpid/...) — legacy's
    # STANDARD_LIBS included -lnosys explicitly. A plain target_link_libraries
    # entry lands too early: libc/libgcc are appended by the compiler driver
    # itself (from --specs=nano.specs) after everything CMake puts on the
    # link line, so nosys has to come in the same way, via a second specs
    # file, to land after them too.
    --specs=nosys.specs
    -T${LINKER_SCRIPT}
)

endif()

#___oOo___
