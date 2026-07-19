# blinky/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# --- Bare-metal cross toolchain (inline, no toolchain file needed for this scale)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)  # skip host-style link test

project(blinky C)

set(CMSIS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cmsis)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/config/tm4c123gh6pm.ld)

add_executable(blinky.elf
    src/main.c
    src/startup_tm4c123.c
)

target_include_directories(blinky.elf PRIVATE
    ${CMSIS_DIR}/core
    ${CMSIS_DIR}/device/Include
)

target_compile_definitions(blinky.elf PRIVATE TM4C123GH6PM)

set(CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

target_compile_options(blinky.elf PRIVATE
    ${CPU_FLAGS}
    -Os -g -Wall -Wextra
    -ffunction-sections -fdata-sections
)

target_link_options(blinky.elf PRIVATE
    ${CPU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=$<TARGET_FILE_DIR:blinky.elf>/blinky.map
    --specs=nosys.specs
)

# Relink if the linker script changes (CMake doesn't track -T automatically)
set_target_properties(blinky.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# .bin for raw flashing + size report after every build
add_custom_command(TARGET blinky.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:blinky.elf> blinky.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:blinky.elf>
)
