# The real flight-controller firmware — implements Main::setup()/loop() and
# links against Target/Main for the entry point, same shape as
# Target/Base/test's delayus. TARGET_MEMORY=ROM (legacy), unlike delayus'
# RAM placement.
if(NOT EMB_HOST)

emb_module(
    NAME  TurnigyHAL
    TITLE "Turnigy flight-controller firmware"
)

set(LINKER_SCRIPT_SRC ${CMAKE_SOURCE_DIR}/application/Target/Main/config/tm4c123gh6pm.ld)
set(LINKER_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/tm4c123gh6pm.rom.ld)

add_custom_command(
    OUTPUT  ${LINKER_SCRIPT}
    COMMAND ${CMAKE_C_COMPILER} -E -P -x c -DLOAD_TO_ROM ${LINKER_SCRIPT_SRC} -o ${LINKER_SCRIPT}
    DEPENDS ${LINKER_SCRIPT_SRC}
    COMMENT "Preprocessing tm4c123gh6pm.ld for ROM placement"
)
add_custom_target(TurnigyHAL_linker_script DEPENDS ${LINKER_SCRIPT})

emb_executable(
    NAME    TurnigyHAL
    SOURCES src/TurnigyHAL.cpp
)
add_dependencies(TurnigyHAL TurnigyHAL_linker_script)
set_target_properties(TurnigyHAL PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# Same reference-cycle reasoning as Target/Base/test/delayus: main()/ResetISR
# reference each other across static libs, and Base needs tivaware's
# driverlib, so every one of our own static libs has to be inside the group
# explicitly or it lands outside and stays unresolved.
target_link_libraries(TurnigyHAL
    PRIVATE
        -Wl,--start-group
        emb::Main
        emb::MainLoop
        emb::PulseModulation
        emb::MPU6050
        emb::IMU
        emb::PID
        emb::LogicUART
        emb::LogicI2C
        emb::Host
        emb::Queue
        emb::SampleStatistics
        emb::Timer
        emb::UART
        emb::PWM
        emb::I2C
        emb::GPIO
        emb::Base
        emb::Filter
        emb::Utils
        emb::Interrupts
        emb::SystemViewOff
        emb::ErrorCodes
        emb::Endianness
        ${CMAKE_SOURCE_DIR}/tivaware/driverlib/gcc/libdriver.a
        # CMSIS-DSP (arm_mat_*/arm_pid_*) — prebuilt, matching legacy's
        # BOARD_LIBS = driver arm_cortexM4lf_math. cmsis/dsp/ only has
        # headers + source, no build; cmsis/tm4c/ has the matching prebuilt
        # archive for this exact core/fpu/toolchain combo.
        ${CMAKE_SOURCE_DIR}/cmsis/tm4c/Lib/GCC/libarm_cortexM4lf_math.a
        -Wl,--end-group
)

# See Target/Base/test/CMakeLists.txt for why -fatal-warnings is dropped and
# nosys.specs is needed for _kill/_getpid/_exit.
target_link_options(TurnigyHAL PRIVATE
    -nostartfiles
    -Wl,--gc-sections
    -Wl,--no-undefined
    -Wl,--undefined=ResetISR
    --specs=nosys.specs
    -T${LINKER_SCRIPT}
)

endif()

#___oOo___
