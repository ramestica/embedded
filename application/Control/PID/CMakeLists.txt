# PID.h pulls in Target/Target.h directly, so this is cortex-m4 only.
if(NOT EMB_HOST)

emb_module(
    NAME  PID
    TITLE "PID and cascade-PID controllers"
)

emb_library(
    NAME    PID
    SOURCES
        src/PID.cpp
        src/CascadePID.cpp
)

target_link_libraries(PID
    PUBLIC
        emb::Base
        emb::cmsis-dsp
)

endif()

#___oOo___
