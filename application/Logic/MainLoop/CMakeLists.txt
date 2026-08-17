if(NOT EMB_HOST)

emb_module(
    NAME  MainLoop
    TITLE "Application main loop: STATS command, telemetry dispatch"
)

emb_library(
    NAME    MainLoop
    SOURCES src/MainLoop.cpp
)

# MainLoop.h itself includes Target/Timer.h too, alongside ErrorCodes/Host/SampleStatistics.
target_link_libraries(MainLoop
    PUBLIC
        emb::ErrorCodes
        emb::Host
        emb::SampleStatistics
        emb::Timer
    PRIVATE
        emb::Utils
        emb::GPIO
        emb::LogicUART
        emb::LogicI2C
)

endif()

#___oOo___
