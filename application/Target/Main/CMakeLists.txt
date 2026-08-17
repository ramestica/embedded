# Provides the real int main(void) entry point, which calls the
# RAM::Target::Main::setup()/loop() hooks — implemented by whichever
# executable links against this (see Target/Base/test's delayus, once
# ported). init.cpp is nano328p-only and not part of Main_SOURCES upstream.
if(NOT EMB_HOST)

emb_module(
    NAME  Main
    TITLE "Firmware entry point (main), dispatches to Main::setup()/loop()"
)

emb_library(
    NAME    Main
    SOURCES src/main.cpp
)

target_link_libraries(Main PUBLIC emb::Base)

endif()

#___oOo___
