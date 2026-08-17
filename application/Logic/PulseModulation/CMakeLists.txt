if(NOT EMB_HOST)

emb_module(
    NAME  PulseModulation
    TITLE "PPM/PWM RC pulse decoding and encoding"
)

emb_library(
    NAME    PulseModulation
    SOURCES
        src/PPM.cpp
        src/PWM.cpp
)

# PulseModulation.h itself includes ErrorCodes.h too, alongside Timer/PWM/Host.
target_link_libraries(PulseModulation
    PUBLIC
        emb::Timer
        emb::PWM
        emb::Host
        emb::ErrorCodes
    PRIVATE
        emb::Utils
        emb::GPIO
)

endif()

#___oOo___
