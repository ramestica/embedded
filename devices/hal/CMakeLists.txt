#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

set(I2C_HAL "" CACHE STRING "I2C HAL backend: tm4c123 | stm32f1")
set_property(CACHE I2C_HAL PROPERTY STRINGS tm4c123 stm32f1)

set(VENDOR ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor)
set(CMSIS  ${CMAKE_CURRENT_SOURCE_DIR}/../../cmsis)

if(I2C_HAL STREQUAL "tm4c123")
    add_library(i2c-hal STATIC
        tm4c123/src/i2c_hal_tm4c.c
        ${VENDOR}/tivaware/driverlib/i2c.c
    )
    target_include_directories(i2c-hal PUBLIC
        ${VENDOR}/tivaware
        ${CMSIS}/core
        ${CMSIS}/device/tm4c/Include
    )
    target_compile_definitions(i2c-hal PUBLIC TM4C123GH6PM)

elseif(I2C_HAL STREQUAL "stm32f1")
    add_library(i2c-hal STATIC
        stm32f1/src/i2c_hal_stm32.c
    )
    target_include_directories(i2c-hal PUBLIC
        ${VENDOR}/stm32-ll
        ${CMSIS}/core
        ${CMSIS}/device/stm32f1/Include
    )
    target_compile_definitions(i2c-hal PUBLIC STM32F103xB)

else()
    message(FATAL_ERROR "I2C_HAL must be set: -DI2C_HAL=tm4c123 (or stm32f1)")
endif()

target_link_libraries(i2c-hal PUBLIC i2c)

#___oOo___
