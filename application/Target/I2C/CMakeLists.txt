# Named plainly "I2C" as the first of two I2C modules ported (Logic/I2C is
# the other); rename this one if that collides once Logic/I2C is ported.
if(NOT EMB_HOST)

emb_module(
    NAME  I2C
    TITLE "TM4C123 I2C driver"
)

emb_library(
    NAME    I2C
    SOURCES src/I2C_tm4c123gh6pm.cpp
)

target_link_libraries(I2C
    PUBLIC
        emb::GPIO
    PRIVATE
        emb::ErrorCodes
        emb::Utils
)

endif()

#___oOo___
