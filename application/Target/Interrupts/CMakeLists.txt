# NVIC vector-table relocation and IRQ enable/disable — cortex-m4 only.
if(NOT EMB_HOST)

emb_module(
    NAME  Interrupts
    TITLE "NVIC vector table relocation and IRQ control"
)

emb_library(
    NAME    Interrupts
    SOURCES src/Interrupts.cpp
)

target_link_libraries(Interrupts PUBLIC emb::Base)

endif()

#___oOo___
