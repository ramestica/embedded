emb_module(
    NAME  Utils
    TITLE "Delay, heap, and bit-index helpers"
)

set(UTILS_SOURCES src/getBitIndex.cpp)

# delayus.cpp (ARM inline asm) and malloc.cpp (bare-metal _sbrk/heap
# overrides) only make sense against newlib on real hardware, not a hosted
# glibc build.
if(NOT EMB_HOST)
    list(APPEND UTILS_SOURCES src/delayus.cpp src/malloc.cpp)
endif()

emb_library(
    NAME    Utils
    SOURCES ${UTILS_SOURCES}
)

target_link_libraries(Utils PUBLIC emb::ErrorCodes)

#___oOo___
