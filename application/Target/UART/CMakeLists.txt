if(NOT EMB_HOST)

emb_module(
    NAME  UART
    TITLE "TM4C123 UART driver"
)

emb_library(
    NAME    UART
    SOURCES src/UART.cpp
)

target_link_libraries(UART
    PUBLIC
        emb::GPIO
    PRIVATE
        emb::Interrupts
)

endif()

#___oOo___
