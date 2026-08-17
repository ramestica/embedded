# Startup/system-init code and the CMSIS device header for TM4C123 — inherently
# bare-metal, so this module is cortex-m4 only (no host build).
if(NOT EMB_HOST)

emb_module(
    NAME  Base
    TITLE "TM4C123 startup code, CMSIS device header, and clock init"
)

emb_library(
    NAME    Base
    SOURCES
        src/startup_tm4c123gh6pm.c
        src/system_tm4c123gh6pm.c
        src/globals.cpp
)

target_link_libraries(Base
    PUBLIC
        emb::cmsis-core
        emb::SystemViewOff
    PRIVATE
        emb::tivaware
)

endif()

#___oOo___
