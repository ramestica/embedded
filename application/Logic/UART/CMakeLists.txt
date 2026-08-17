# CMake target named "LogicUART" (header stays Logic/UART.h) since
# Target/UART already claimed the plain "UART" target name — CMake target
# names are global, unlike our per-directory header namespacing.
if(NOT EMB_HOST)

emb_module(
    NAME  LogicUART
    TITLE "Host-facing UART protocol layer"
)

# SerialStreamer.cpp exists alongside UART.cpp but isn't in the legacy
# UART_SOURCES — left out of this port to match.
emb_library(
    NAME    LogicUART
    SOURCES src/UART.cpp
)

# All PUBLIC: Logic/UART.h itself includes each of these headers directly.
target_link_libraries(LogicUART
    PUBLIC
        emb::Utils
        emb::UART
        emb::Host
        emb::Interrupts
        emb::ErrorCodes
)

endif()

#___oOo___
