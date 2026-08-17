if(NOT EMB_HOST)

emb_module(
    NAME  Timer
    TITLE "TM4C123 timer driver and SysTick"
)

emb_library(
    NAME    Timer
    SOURCES
        src/Timer.cpp
        src/Timer_tm4c123gh6pm.cpp
        src/SystemTick_tm4c123gh6pm.cpp
)

target_link_libraries(Timer
    PUBLIC
        emb::GPIO
    PRIVATE
        emb::Interrupts
)

endif()

#___oOo___
