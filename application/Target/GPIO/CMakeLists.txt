if(NOT EMB_HOST)

emb_module(
    NAME  GPIO
    TITLE "TM4C123 GPIO driver"
)

emb_library(
    NAME    GPIO
    SOURCES
        src/GPIO.cpp
        src/GPIO_tm4c123gh6pm.cpp
)

target_link_libraries(GPIO
    PUBLIC
        emb::Base
    PRIVATE
        emb::Utils
        emb::Interrupts
)

endif()

#___oOo___
