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

# The legacy linker script (Target/Main/config/tm4c123gh6pm.ld) never defines
# an .ARM.exidx output section, so GCC's stack-unwind tables land as an
# orphan section that overlaps .data. Already -fno-exceptions; these two
# suppress unwind-table generation entirely instead of patching the script.
if(NOT EMB_HOST)
    add_compile_options(-fno-unwind-tables -fno-asynchronous-unwind-tables)
endif()

#
# Support
#
add_subdirectory(Support/ErrorCodes)
add_subdirectory(Support/Utils)

#
# Target
#
add_subdirectory(Target/SystemView)
add_subdirectory(Target/Base)
add_subdirectory(Target/Interrupts)
add_subdirectory(Target/GPIO)
add_subdirectory(Target/Timer)
add_subdirectory(Target/UART)
add_subdirectory(Target/PWM)
add_subdirectory(Target/I2C)
add_subdirectory(Target/Main)
add_subdirectory(Target/Base/test)

#
# Logic
#
add_subdirectory(Logic/Queue)
add_subdirectory(Logic/SampleStatistics)
add_subdirectory(Logic/Endianness)
add_subdirectory(Logic/Host)
add_subdirectory(Logic/UART)
add_subdirectory(Logic/I2C)
add_subdirectory(Logic/PulseModulation)
add_subdirectory(Logic/MainLoop)

#
# Sensors
#
add_subdirectory(Sensors/MPU6050)

#
# Control
#
add_subdirectory(Control/Filter)
add_subdirectory(Control/IMU)
add_subdirectory(Control/PID)

#
# Drones
#
add_subdirectory(Drones/TurnigyHAL)

#___oOo___
